param(
    [switch]$IncludeOptional,
    [switch]$FullBuild
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Baseline build smoke'

$RepoRoot = Get-UsagiRoot

Invoke-TestStep 'build scripts and checked-in build tools exist' {
    Assert-PathExists (Join-Path $RepoRoot 'Tools\build\generator.rb')
    Assert-PathExists (Join-Path $RepoRoot 'Tools\build\game_generator.rb')
    Assert-PathExists (Join-Path $RepoRoot 'Tools\build\build_config.rb')
    Assert-PathExists (Join-Path $RepoRoot 'Tools\bin\ninja.exe')
}

$ruby = Get-Command ruby -ErrorAction SilentlyContinue
if ($ruby) {
    Invoke-TestStep 'core Ruby build scripts pass syntax checks' {
        foreach ($path in @(
            'Tools\build\generator.rb',
            'Tools\build\game_generator.rb',
            'Tools\build\generator_util.rb',
            'Tools\build\build_config.rb',
            'Tools\ruby\combine_yaml.rb',
            'Tools\ruby\expand_erb.rb'
        )) {
            & ruby -c (Join-Path $RepoRoot $path) | Out-Null
            if ($LASTEXITCODE -ne 0) {
                throw "Ruby syntax check failed: $path"
            }
        }
    }
}
else {
    Add-TestSkip 'Ruby is not on PATH; skipped build script syntax checks.'
}

if ($FullBuild -or $IncludeOptional) {
    $rake = Get-Command rake -ErrorAction SilentlyContinue
    if ($rake) {
        Invoke-TestStep 'rake can generate Windows debug projects' {
            Push-Location $RepoRoot
            try {
                & rake platform=win build=debug projects
                if ($LASTEXITCODE -ne 0) {
                    throw "rake projects failed with exit code $LASTEXITCODE"
                }
            }
            finally {
                Pop-Location
            }
        }
    }
    else {
        Add-TestSkip 'rake is not on PATH; skipped optional project generation.'
    }
}
else {
    Add-TestSkip 'Full build/project generation is opt-in; pass -FullBuild or -IncludeOptional.'
}

Complete-TestHarness
