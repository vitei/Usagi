param(
    [switch]$IncludeOptional
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Release readiness smoke'

$RepoRoot = Get-UsagiRoot
$TestsRoot = Get-TestsRoot

$Steps = @(
    'BaselineBuild',
    'BehaviorTreeConversion',
    'LevelConversion',
    'EntityHierarchy',
    'ResourcePakExporter',
    'ShaderPackageRendering'
)

$DeferredSteps = @(
    'AudioToolReverseEngineering',
    'AudioToolBuildRules',
    'PreviewHost',
    'ModelInstancing',
    'ResourceAsyncFoundation',
    'StableEntityHandles'
)

foreach ($step in $Steps) {
    Invoke-TestStep "release readiness step exists: $step" {
        Assert-PathExists (Join-Path $TestsRoot "$step\Run.ps1")
    }
}

if ($IncludeOptional) {
    foreach ($step in $Steps) {
        Invoke-TestStep "release readiness step runs: $step" {
            $script = Join-Path $TestsRoot "$step\Run.ps1"
            & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $script
            if ($LASTEXITCODE -ne 0) {
                throw "$step failed with exit code $LASTEXITCODE"
            }
        }
    }
}
else {
    Add-TestSkip 'Aggregated readiness execution is opt-in; RunAll.ps1 already invokes each suite once.'
}

foreach ($step in $DeferredSteps) {
    $script = Join-Path $TestsRoot "$step\Run.ps1"
    if (Test-Path -LiteralPath $script) {
        Add-TestSkip "Deferred suite available but not part of PR 02 default readiness: $step"
    }
    else {
        Add-TestSkip "Deferred suite not present in this branch: $step"
    }
}

Complete-TestHarness
