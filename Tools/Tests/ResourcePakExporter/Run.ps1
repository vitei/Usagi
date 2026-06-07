param(
    [switch]$IncludeOptional,
    [switch]$RunNative
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Resource pak exporter smoke'

$RepoRoot = Get-UsagiRoot
$PakFileGen = Join-Path $RepoRoot 'Tools\bin\PakFileGen.exe'

Invoke-TestStep 'pak exporter binary and texture pak fixture exist' {
    Assert-PathExists $PakFileGen
    Assert-PathExists (Join-Path $RepoRoot 'Data\Textures\EngineTextures.yml')
}

Invoke-TestStep 'pak exporter CLI responds to invalid invocation' {
    Invoke-ExternalCommand -FilePath $PakFileGen -ExpectedExitCodes @(-1) -TimeoutSeconds 5 | Out-Null
}

if ($RunNative -or $IncludeOptional) {
    Invoke-TestStep 'optional texture pak generation smoke' {
        $workspace = New-TestWorkspace 'ResourcePakExporter'
        $output = Join-Path $workspace 'EngineTextures.pak'
        $temp = Join-Path $workspace 'tmp'
        New-Item -ItemType Directory -Force -Path $temp | Out-Null
        Invoke-ExternalCommand `
            -FilePath $PakFileGen `
            -Arguments @((Join-Path $RepoRoot 'Data\Textures\EngineTextures.yml'), "-o$output", "-t$temp", '-def') `
            -WorkingDirectory $RepoRoot `
            -ExpectedExitCodes @(0) `
            -TimeoutSeconds 30 | Out-Null
        Assert-PathExists $output
    }
}
else {
    Add-TestSkip 'Native pak generation is opt-in; pass -RunNative or -IncludeOptional.'
}

Complete-TestHarness
