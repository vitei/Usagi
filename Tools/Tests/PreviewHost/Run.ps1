param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [ValidateSet("x64")]
    [string]$Platform = "x64",
    [int]$TimeoutSeconds = 20,
    [switch]$SkipBuild,
    [switch]$SkipAssetLoads,
    [switch]$PreflightOnly
)

$ErrorActionPreference = "Stop"

if ([Threading.Thread]::CurrentThread.GetApartmentState() -ne [Threading.ApartmentState]::STA) {
    $powershell = (Get-Command powershell.exe).Source
    $args = @(
        "-STA",
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", $PSCommandPath,
        "-Configuration", $Configuration,
        "-Platform", $Platform,
        "-TimeoutSeconds", $TimeoutSeconds
    )

    if ($SkipBuild) {
        $args += "-SkipBuild"
    }

    if ($SkipAssetLoads) {
        $args += "-SkipAssetLoads"
    }

    if ($PreflightOnly) {
        $args += "-PreflightOnly"
    }

    & $powershell @args
    exit $LASTEXITCODE
}

function Get-RepoRoot {
    $root = (Resolve-Path $PSScriptRoot).Path
    while ($root -and !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        $parent = Split-Path $root -Parent
        if ($parent -eq $root) {
            break
        }

        $root = $parent
    }

    if (!$root -or !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        throw "Could not locate Usagi repository root from $PSScriptRoot"
    }

    return $root
}

function Find-MSBuild {
    $fromPath = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $found = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
        if ($found) {
            return $found
        }
    }

    return $null
}

function Get-MSBuild {
    $found = Find-MSBuild
    if ($found) {
        return $found
    }

    throw "MSBuild.exe was not found. Install Visual Studio Build Tools with C++ workload or run from a Developer PowerShell."
}

function New-PreviewPrerequisite {
    param(
        [string]$Name,
        [string]$Path,
        [bool]$Present,
        [bool]$Required,
        [string]$Action
    )

    [PSCustomObject]@{
        Name = $Name
        Path = $Path
        Present = $Present
        Required = $Required
        Action = $Action
    }
}

function Write-PreviewPreflight {
    param([object[]]$Items)

    Write-Host "Preview host preflight:"
    foreach ($item in $Items) {
        $state = if ($item.Present) { "OK" } elseif ($item.Required) { "MISSING" } else { "optional" }
        Write-Host ("[{0}] {1}: {2}" -f $state, $item.Name, $item.Path)
        if (!$item.Present -and $item.Action) {
            Write-Host ("      {0}" -f $item.Action)
        }
    }
}

function Assert-PreviewPreflight {
    param([object[]]$Items)

    $missing = @($Items | Where-Object { $_.Required -and !$_.Present })
    if ($missing.Count -eq 0) {
        return
    }

    Write-PreviewPreflight $Items
    $names = ($missing | ForEach-Object { $_.Name }) -join ", "
    throw "Preview host prerequisites are missing: $names"
}

function Send-Json([System.Diagnostics.Process]$Process, [string]$Json) {
    $Process.StandardInput.WriteLine($Json)
    $Process.StandardInput.Flush()
}

function Wait-ForLine {
    param(
        [System.Diagnostics.Process]$Process,
        [scriptblock]$Predicate,
        [string]$Description,
        [int]$TimeoutSeconds
    )

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    $seen = New-Object System.Collections.Generic.List[string]
    $readTask = $null

    while ([DateTime]::UtcNow -lt $deadline) {
        [System.Windows.Forms.Application]::DoEvents()

        if ($null -eq $readTask) {
            $readTask = $Process.StandardOutput.ReadLineAsync()
        }

        if ($readTask.Wait(50)) {
            $line = $readTask.Result
            $readTask = $null
            if ($null -eq $line) {
                break
            }

            $seen.Add($line)
            if (& $Predicate $line) {
                return $line
            }
        }

        if ($Process.HasExited) {
            break
        }
    }

    $exitDetails = ""
    if ($Process.HasExited) {
        $exitDetails = "Process exited with code $($Process.ExitCode)."
    }

    $stderr = ""
    if ($Process.HasExited) {
        $stderr = $Process.StandardError.ReadToEnd()
    }

    throw "Timed out waiting for $Description. $exitDetails`nstdout:`n$($seen -join "`n")`nstderr:`n$stderr"
}

function Copy-PreviewResource([string]$RelativePath) {
    $source = Join-Path $repoRoot "Data\$RelativePath"
    if (!(Test-Path $source)) {
        return
    }

    $destination = Join-Path $romfilesPath $RelativePath
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination -Recurse -Force
}

function Ensure-PreviewModelResource {
    $modelPath = Join-Path $romfilesPath "Models\PBRSample\PBRSample.vmdf"
    if (Test-Path $modelPath) {
        return
    }

    $ayataka = Join-Path $repoRoot "Tools\bin\Ayataka.exe"
    if (!(Test-Path $ayataka)) {
        throw "Preview smoke requires Ayataka.exe to build the model fixture: $ayataka"
    }

    $modelSource = Join-Path $repoRoot "Data\Models\PBRSample\PBRSample.fbx"
    if (!(Test-Path $modelSource)) {
        throw "Preview smoke requires model source fixture: $modelSource"
    }

    $modelOutDir = Split-Path -Parent $modelPath
    $skeletonPath = Join-Path $repoRoot "_build\skel\PBRSample\PBRSample.vmdf.xml"
    New-Item -ItemType Directory -Force -Path $modelOutDir, (Split-Path -Parent $skeletonPath) | Out-Null

    & $ayataka `
        "-a16" `
        "-o$modelPath" `
        "-sk$modelOutDir\" `
        "-lh" `
        "-d$modelPath.d" `
        "-h$skeletonPath" `
        $modelSource
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (!(Test-Path $modelPath)) {
        throw "Ayataka did not produce the preview model fixture: $modelPath"
    }
}

$repoRoot = Get-RepoRoot
$env:USAGI_DIR = $repoRoot

$projectPath = Join-Path $repoRoot "Tools\Source\UsagiPreviewHost\project\UsagiPreviewHost.vcxproj"
$hostPath = Join-Path $repoRoot "Tools\bin\UsagiPreviewHost.exe"
$generatedProjects = Join-Path $repoRoot "_build\projects"
$romfilesPath = Join-Path $repoRoot "_romfiles\win"
$nameDataHash = Join-Path $romfilesPath "nameDataHash.bin"
$ayataka = Join-Path $repoRoot "Tools\bin\Ayataka.exe"
$modelSource = Join-Path $repoRoot "Data\Models\PBRSample\PBRSample.fbx"
$particleData = Join-Path $repoRoot "Data\Particle"
$effectsData = Join-Path $repoRoot "Data\GLSL\effects"
$texturesData = Join-Path $repoRoot "Data\Textures"
$msbuildPath = Find-MSBuild
$msbuildDisplayPath = if ($msbuildPath) { $msbuildPath } else { "msbuild.exe" }

$preflight = @(
    New-PreviewPrerequisite "Preview host project" $projectPath (Test-Path $projectPath) $true "The PR12 preview host project file is missing."
    New-PreviewPrerequisite "Generated native projects" $generatedProjects (Test-Path $generatedProjects) (!$SkipBuild) "Run project generation before building the preview host."
    New-PreviewPrerequisite "MSBuild" $msbuildDisplayPath ($null -ne $msbuildPath) (!$SkipBuild) "Install Visual Studio Build Tools with the C++ workload or run from Developer PowerShell."
    New-PreviewPrerequisite "Preview host executable" $hostPath (Test-Path $hostPath) $SkipBuild "Build UsagiPreviewHost.exe, or rerun without -SkipBuild."
    New-PreviewPrerequisite "nameDataHash.bin" $nameDataHash (Test-Path $nameDataHash) $true "Run a data build before launching the preview host smoke."
    New-PreviewPrerequisite "Particle source data" $particleData (Test-Path $particleData) (!$SkipAssetLoads) "Particle preview smoke needs Data\Particle fixtures."
    New-PreviewPrerequisite "Shader effect data" $effectsData (Test-Path $effectsData) (!$SkipAssetLoads) "Particle/model preview smoke needs shader effect fixtures."
    New-PreviewPrerequisite "Texture source data" $texturesData (Test-Path $texturesData) (!$SkipAssetLoads) "Preview smoke copies Data\Textures into romfiles."
    New-PreviewPrerequisite "Ayataka model converter" $ayataka (Test-Path $ayataka) (!$SkipAssetLoads) "Model preview smoke needs Ayataka.exe to build the PBRSample fixture."
    New-PreviewPrerequisite "PBRSample model source" $modelSource (Test-Path $modelSource) (!$SkipAssetLoads) "Model preview smoke needs Data\Models\PBRSample\PBRSample.fbx."
)

if ($PreflightOnly) {
    Write-PreviewPreflight $preflight
    exit 0
}

Assert-PreviewPreflight $preflight

if (!$SkipBuild) {
    $msbuild = Get-MSBuild
    & $msbuild $projectPath /m /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

if (!(Test-Path $hostPath)) {
    throw "Preview host executable was not found: $hostPath"
}

Add-Type -AssemblyName System.Windows.Forms

$form = New-Object System.Windows.Forms.Form
$form.Text = "Usagi Preview Host Smoke"
$form.Width = 640
$form.Height = 480
$form.Show()
[System.Windows.Forms.Application]::DoEvents()

New-Item -ItemType Directory -Force -Path $romfilesPath | Out-Null

if (!(Test-Path $nameDataHash)) {
    throw "Preview smoke requires nameDataHash.bin at $nameDataHash. Run a data build first."
}

if (!$SkipAssetLoads) {
    Copy-PreviewResource "Particle"
    Copy-PreviewResource "Effects"
    Copy-PreviewResource "Textures"
    Copy-PreviewResource "Models\PBRSample"
    Ensure-PreviewModelResource
}

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $hostPath
$startInfo.WorkingDirectory = $romfilesPath
$startInfo.UseShellExecute = $false
$startInfo.CreateNoWindow = $true
$startInfo.RedirectStandardInput = $true
$startInfo.RedirectStandardOutput = $true
$startInfo.RedirectStandardError = $true

$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo

try {
    [void]$process.Start()

    Send-Json $process '{"type":"pick","x":1,"y":2}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"error"' -and $line -match 'not been initialized' } "pre-init protocol error" $TimeoutSeconds)

    $dataPath = (Join-Path $repoRoot "Data").Replace("\", "\\")
    $escapedRomfilesPath = $romfilesPath.Replace("\", "\\")
    Send-Json $process "{`"type`":`"init`",`"protocolVersion`":1,`"dataPath`":`"$dataPath`",`"romfilesPath`":`"$escapedRomfilesPath`"}"
    [void](Wait-ForLine $process { param($line) $line -match '"type":"ready"' -and $line -match '"protocolVersion":1' } "ready response" $TimeoutSeconds)

    $hwnd = $form.Handle.ToInt64()
    Send-Json $process "{`"type`":`"attachWindow`",`"hwnd`":$hwnd,`"width`":640,`"height`":480}"
    [void](Wait-ForLine $process { param($line) $line -match 'Usagi engine initialized for preview host' } "engine initialization diagnostic" $TimeoutSeconds)

    if (!$SkipAssetLoads) {
        $entityDir = Join-Path $repoRoot "_romfiles\preview"
        New-Item -ItemType Directory -Force -Path $entityDir | Out-Null
        $entityPath = Join-Path $entityDir "PreviewPBRSample.yml"
        @'
ModelComponent:
  name: PBRSample/PBRSample.vmdf
'@ | Set-Content -Encoding ASCII -Path $entityPath

        $escapedEntityPath = $entityPath.Replace("\", "\\")
        Send-Json $process "{`"type`":`"loadEntity`",`"path`":`"$escapedEntityPath`"}"
        [void](Wait-ForLine $process { param($line) $line -match '"type":"loaded"' -and $line -match '"resourceType":"entity"' -and $line -match '"success":true' } "successful entity load response" $TimeoutSeconds)

        Send-Json $process '{"type":"loadParticle","emitterPath":"Particle/white_circle.pem","effectPath":null}'
        [void](Wait-ForLine $process { param($line) $line -match '"type":"loaded"' -and $line -match '"resourceType":"particle"' } "particle load response" $TimeoutSeconds)
    }

    Send-Json $process '{"type":"tick","deltaTime":0.0166667}'
    Start-Sleep -Milliseconds 250
    [System.Windows.Forms.Application]::DoEvents()

    Send-Json $process '{"type":"pick","x":4,"y":8}'
    [void](Wait-ForLine $process { param($line) $line -match '"type":"picked"' -and $line -match '"entityId":null' } "empty pick response" $TimeoutSeconds)

    Send-Json $process '{"type":"shutdown"}'
    if (!$process.WaitForExit($TimeoutSeconds * 1000)) {
        $process.Kill()
        throw "Preview host did not exit after shutdown."
    }

    if ($process.ExitCode -ne 0) {
        $stderr = $process.StandardError.ReadToEnd()
        throw "Preview host exited with code $($process.ExitCode).`nstderr:`n$stderr"
    }

    Write-Host "Preview host asset smoke passed."
}
finally {
    if (!$process.HasExited) {
        try {
            Send-Json $process '{"type":"shutdown"}'
            if (!$process.WaitForExit(1000)) {
                $process.Kill()
            }
        }
        catch {
            try { $process.Kill() } catch { }
        }
    }

    $process.Dispose()
    $form.Close()
    $form.Dispose()
}
