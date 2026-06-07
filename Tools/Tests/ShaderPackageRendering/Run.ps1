param(
    [switch]$IncludeOptional,
    [switch]$RunNative
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Shader package rendering smoke'

$RepoRoot = Get-UsagiRoot
$ShaderPackage = Join-Path $RepoRoot 'Tools\bin\ShaderPackage.exe'

Invoke-TestStep 'shader package binary and effect fixtures exist' {
    Assert-PathExists $ShaderPackage
    foreach ($path in @(
        'Data\GLSL\effects\Debug.yml',
        'Data\GLSL\effects\Model.yml',
        'Data\GLSL\effects\PostProcess.yml',
        'Data\GLSL\effects\Particles.yml',
        'Data\GLSL\shaders'
    )) {
        Assert-PathExists (Join-Path $RepoRoot $path)
    }
}

Invoke-TestStep 'shader package CLI responds to invalid invocation' {
    Invoke-ExternalCommand -FilePath $ShaderPackage -ExpectedExitCodes @(-1) -TimeoutSeconds 5 | Out-Null
}

if ($RunNative -or $IncludeOptional) {
    Invoke-TestStep 'optional debug shader package generation smoke' {
        $workspace = New-TestWorkspace 'ShaderPackageRendering'
        $output = Join-Path $workspace 'Debug.pak'
        $temp = Join-Path $workspace 'tmp'
        New-Item -ItemType Directory -Force -Path $temp | Out-Null
        Invoke-ExternalCommand `
            -FilePath $ShaderPackage `
            -Arguments @((Join-Path $RepoRoot 'Data\GLSL\effects\Debug.yml'), "-o$output", "-t$temp", '-sData\GLSL\shaders', '-avulkan') `
            -WorkingDirectory $RepoRoot `
            -ExpectedExitCodes @(0) `
            -TimeoutSeconds 30 | Out-Null
        Assert-PathExists $output
    }
}
else {
    Add-TestSkip 'Native shader package generation is opt-in; pass -RunNative or -IncludeOptional.'
}

Complete-TestHarness
