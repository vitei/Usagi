param(
    [switch]$Launch,
    [switch]$RequireValidation,
    [int]$SmokeSeconds = 2
)

$ErrorActionPreference = 'Stop'

function Find-ToolsRoot {
    param([string]$StartPath)

    $Current = (Resolve-Path $StartPath).Path
    while ($Current) {
        $Candidate = Join-Path $Current 'tools\usagi-dev-env.ps1'
        if (Test-Path $Candidate) {
            return (Join-Path $Current 'tools')
        }

        $Parent = Split-Path -Parent $Current
        if ($Parent -eq $Current) {
            break
        }
        $Current = $Parent
    }

    throw "Unable to find tools\usagi-dev-env.ps1 above $StartPath."
}

function Invoke-Checked {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FilePath exited with code $LASTEXITCODE."
    }
}

$TestDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$UsagiRoot = (Resolve-Path (Join-Path $TestDir '..\..\..')).Path
$ToolsRoot = Find-ToolsRoot -StartPath $UsagiRoot
$EnvScript = Join-Path $ToolsRoot 'usagi-dev-env.ps1'

. $EnvScript

# The shared env script may be anchored to a different checkout. Tests in
# worktrees must use the active checkout for source and generated protobuf paths.
$env:USAGI_DIR = $UsagiRoot

$ValidationSdk = Get-ChildItem $ToolsRoot -Directory -Filter 'VulkanSDK*' -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending |
    Where-Object { Test-Path (Join-Path $_.FullName 'Bin\VkLayer_khronos_validation.json') } |
    Select-Object -First 1

if ($ValidationSdk) {
    $env:VK_LAYER_PATH = Join-Path $ValidationSdk.FullName 'Bin'
    $env:PATH = "$env:VK_LAYER_PATH;$env:PATH"
}
elseif ($RequireValidation) {
    throw "VK_LAYER_KHRONOS_validation is not staged under $ToolsRoot."
}

$BuildRoot = Join-Path $ToolsRoot 'test-build\ParticleEditorSmoke'
$RuntimeRoot = Join-Path $BuildRoot 'romfiles\win'
$EffectsRoot = Join-Path $RuntimeRoot 'Effects'
$TexturesRoot = Join-Path $RuntimeRoot 'Textures'
$ParticleRoot = Join-Path $RuntimeRoot 'Particle'
$ParticleEmittersRoot = Join-Path $ParticleRoot 'Emitters'
$ParticleEffectsRoot = Join-Path $ParticleRoot 'Effects'
$ShaderTemp = Join-Path $BuildRoot 'shader-temp'
$ShaderDir = Join-Path $UsagiRoot 'Data\GLSL\shaders'
$ShaderPackage = Join-Path $UsagiRoot 'Tools\bin\ShaderPackage.exe'
$ParticleEditorProject = Join-Path $UsagiRoot 'Tools\Source\ParticleEditor\project\ParticleEditor.vcxproj'
$ParticleEditorExe = Join-Path $UsagiRoot 'Tools\bin\ParticleEditor.exe'
$RubyPbDir = Join-Path $UsagiRoot '_build\ruby'
$SharedRubyPbDir = Join-Path (Split-Path -Parent $ToolsRoot) 'Usagi\_build\ruby'
$MSBuild = if ($env:MSBUILD_DIR) { Join-Path $env:MSBUILD_DIR 'MSBuild.exe' } else { $null }

if (-not (Test-Path $ShaderPackage)) {
    throw "ShaderPackage.exe not found: $ShaderPackage"
}

Push-Location $UsagiRoot
try {
    $RubyPbMarker = Join-Path $RubyPbDir 'Engine\Particles\Scripted\ScriptEmitter.pb.rb'
    if (-not (Test-Path $RubyPbMarker)) {
        try {
            Invoke-Checked -FilePath 'rake' -Arguments @('platform=win', 'build=debug', 'projects')
        }
        catch {
            $ProjectGenerationError = $_
        }
    }
}
finally {
    Pop-Location
}

if (-not (Test-Path (Join-Path $RubyPbDir 'Engine\Particles\Scripted\ScriptEmitter.pb.rb'))) {
    $SharedRubyPbMarker = Join-Path $SharedRubyPbDir 'Engine\Particles\Scripted\ScriptEmitter.pb.rb'
    if ((Test-Path $SharedRubyPbMarker) -and ($SharedRubyPbDir -ne $RubyPbDir)) {
        Write-Warning "Using shared generated Ruby protobufs from $SharedRubyPbDir because local project generation failed: $ProjectGenerationError"
        $RubyPbDir = $SharedRubyPbDir
    }
    else {
        throw "Ruby protobuf output not found after project generation: $RubyPbDir. Generation error: $ProjectGenerationError"
    }
}

$RubyPbRequireDir = $RubyPbDir.Replace('\', '/')

New-Item -ItemType Directory -Force -Path $EffectsRoot, $TexturesRoot, $ParticleRoot, $ParticleEmittersRoot, $ParticleEffectsRoot, $ShaderTemp | Out-Null

Push-Location $UsagiRoot
try {
    $ParticleShaderEffect = Join-Path $UsagiRoot 'Data\GLSL\effects\Particles.yml'
    $ParticleShaderPak = Join-Path $EffectsRoot 'Particles.pak'
    Invoke-Checked -FilePath $ShaderPackage -Arguments @($ParticleShaderEffect, "-o$ParticleShaderPak", "-t$ShaderTemp", "-s$ShaderDir", '-avulkan')

    foreach ($Texture in Get-ChildItem (Join-Path $UsagiRoot 'Data\Textures') -Recurse -File) {
        $Relative = $Texture.FullName.Substring((Join-Path $UsagiRoot 'Data\Textures').Length).TrimStart('\')
        $OutputPath = Join-Path $TexturesRoot $Relative
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
        Copy-Item -LiteralPath $Texture.FullName -Destination $OutputPath -Force
    }

    $ParticleSources = @(
        @{ Source = 'Data/Particle/Emitters/multi_texture_slots.yml'; Output = (Join-Path $ParticleEmittersRoot 'multi_texture_slots.pem') },
        @{ Source = 'Data/Particle/Effects/multi_texture_slots.yml'; Output = (Join-Path $ParticleEffectsRoot 'multi_texture_slots.pfx') }
    )

    foreach ($Particle in $ParticleSources) {
        Invoke-Checked -FilePath 'ruby' -Arguments @('./Tools/ruby/yml2vpb.rb', '-R', $RubyPbRequireDir, '-o', $Particle.Output, $Particle.Source)
    }

    $DataList = Join-Path $RuntimeRoot 'data_list.txt'
    $RuntimeRootResolved = (Resolve-Path $RuntimeRoot).Path
    Get-ChildItem $RuntimeRoot -Recurse -File |
        Where-Object { $_.Name -notin @('nameDataHash.bin', 'data_list.txt') -and $_.Extension -ne '.d' } |
        ForEach-Object { $_.FullName.Substring($RuntimeRootResolved.Length + 1).Replace('\', '/') } |
        Set-Content -Path $DataList -Encoding ASCII

    Invoke-Checked -FilePath 'ruby' -Arguments @('./Tools/ruby/nameDataHashListGen.rb', '-R', $RubyPbRequireDir, '-o', (Join-Path $RuntimeRoot 'nameDataHash.bin'), '-d', $RuntimeRoot, $DataList)
}
finally {
    Pop-Location
}

if ($Launch) {
    if (-not $MSBuild -or -not (Test-Path $MSBuild)) {
        throw 'MSBUILD_DIR is not set. Install Visual Studio Build Tools or update tools\usagi-dev-env.ps1.'
    }

    $SolutionDir = "$UsagiRoot\"
    Invoke-Checked -FilePath $MSBuild -Arguments @($ParticleEditorProject, '/p:Configuration=Debug', '/p:Platform=x64', "/p:SolutionDir=$SolutionDir", '/m', '/nologo')

    if (-not (Test-Path $ParticleEditorExe)) {
        throw "ParticleEditor.exe not found: $ParticleEditorExe"
    }

    $StdOutLog = Join-Path $BuildRoot 'ParticleEditor.stdout.log'
    $StdErrLog = Join-Path $BuildRoot 'ParticleEditor.stderr.log'
    Remove-Item -LiteralPath $StdOutLog, $StdErrLog -Force -ErrorAction SilentlyContinue

    $Process = Start-Process `
        -FilePath $ParticleEditorExe `
        -WorkingDirectory $RuntimeRoot `
        -PassThru `
        -WindowStyle Hidden `
        -RedirectStandardOutput $StdOutLog `
        -RedirectStandardError $StdErrLog
    Start-Sleep -Seconds $SmokeSeconds

    if ($Process.HasExited) {
        $StdErr = if (Test-Path $StdErrLog) { (Get-Content $StdErrLog -Tail 20) -join [Environment]::NewLine } else { '' }
        $StdOut = if (Test-Path $StdOutLog) { (Get-Content $StdOutLog -Tail 20) -join [Environment]::NewLine } else { '' }
        throw "ParticleEditor exited during smoke window with code $($Process.ExitCode).`nSTDERR:`n$StdErr`nSTDOUT:`n$StdOut"
    }

    Stop-Process -Id $Process.Id -Force
    $Process.WaitForExit()
}

Write-Host "Particle multi-texture smoke preflight passed: $RuntimeRoot"
if ($ValidationSdk) {
    Write-Host "Vulkan validation layer path: $env:VK_LAYER_PATH"
}
