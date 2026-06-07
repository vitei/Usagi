param(
    [switch]$IncludeOptional
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Entity hierarchy smoke'

$RepoRoot = Get-UsagiRoot

Invoke-TestStep 'hierarchy processor is present' {
    Assert-PathExists (Join-Path $RepoRoot 'Tools\ruby\process_hierarchy.rb')
}

$ruby = Get-Command ruby -ErrorAction SilentlyContinue
if ($ruby) {
    Invoke-TestStep 'hierarchy processor passes Ruby syntax check' {
        & ruby -c (Join-Path $RepoRoot 'Tools\ruby\process_hierarchy.rb') | Out-Null
        if ($LASTEXITCODE -ne 0) {
            throw "process_hierarchy.rb syntax check failed with exit code $LASTEXITCODE"
        }
    }
}
else {
    Add-TestSkip 'Ruby is not on PATH; skipped hierarchy processor syntax check.'
}

Add-TestSkip 'Full entity hierarchy fixtures are deferred to the entity hierarchy tooling PR.'
Complete-TestHarness
