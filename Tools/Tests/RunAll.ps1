param(
    [switch]$IncludeOptional
)

$ErrorActionPreference = 'Stop'

$TestsRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$Suites = @(
    'BaselineBuild',
    'BehaviorTreeConversion',
    'LevelConversion',
    'EntityHierarchy',
    'ResourcePakExporter',
    'ShaderPackageRendering',
    'ReleaseReadiness',
    'PerformanceBaseline'
)

$failed = @()

foreach ($suite in $Suites) {
    $script = Join-Path $TestsRoot "$suite\Run.ps1"
    if (-not (Test-Path -LiteralPath $script)) {
        $failed += "${suite}: missing $script"
        continue
    }

    Write-Host ""
    Write-Host "## $suite"
    $args = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $script)
    if ($IncludeOptional) {
        $args += '-IncludeOptional'
    }

    & powershell.exe @args
    if ($LASTEXITCODE -ne 0) {
        $failed += "$suite exited with $LASTEXITCODE"
    }
}

if ($failed.Count -gt 0) {
    Write-Host ""
    Write-Host "FAILED SUITES"
    $failed | ForEach-Object { Write-Host $_ }
    exit 1
}

Write-Host ""
Write-Host "All test harness smoke suites completed."
