param(
    [switch]$IncludeOptional,
    [string]$OutputDir
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Performance baseline smoke'

$RepoRoot = Get-UsagiRoot
if (-not $OutputDir) {
    $OutputDir = Join-Path (New-TestWorkspace 'PerformanceBaseline') 'report'
}

Invoke-TestStep 'write baseline metadata report' {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    $report = [PSCustomObject]@{
        generatedAt = [DateTimeOffset]::Now.ToString('o')
        usagiRoot = $RepoRoot
        note = 'PR 02 lightweight harness baseline; heavyweight timing suites are deferred.'
        suites = @(
            'BaselineBuild',
            'BehaviorTreeConversion',
            'LevelConversion',
            'EntityHierarchy',
            'ResourcePakExporter',
            'ShaderPackageRendering'
        )
    }

    $json = Join-Path $OutputDir 'baseline.json'
    $markdown = Join-Path $OutputDir 'baseline.md'
    $report | ConvertTo-Json -Depth 4 | Set-Content -Encoding ASCII -Path $json

    @(
        '# Usagi Lightweight Performance Baseline',
        '',
        "Generated: $($report.generatedAt)",
        '',
        'This PR 02 baseline records the smoke harness surface only.',
        'Timing and render baselines are deferred until the relevant native suites are ported.'
    ) | Set-Content -Encoding ASCII -Path $markdown

    Assert-PathExists $json
    Assert-PathExists $markdown
}

Add-TestSkip 'Historical ECS/render timing commands are outside the PR 02 Tools/Tests-only baseline.'
Complete-TestHarness
