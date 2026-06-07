param(
    [switch]$IncludeOptional
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Level conversion smoke'

$RepoRoot = Get-UsagiRoot

Invoke-TestStep 'level conversion scripts are present' {
    foreach ($path in @(
        'Tools\python\lvl2vhir\lvl2vhir.py',
        'Tools\python\lvl2vhir\Level2Yaml.py',
        'Tools\python\lvl2vhir\Level2Instances.py',
        'Tools\python\lvl2vhir\LevelEditor\Game.py'
    )) {
        Assert-PathExists (Join-Path $RepoRoot $path)
    }
}

$python2 = Get-Command python2 -ErrorAction SilentlyContinue
if (-not $python2) {
    Add-TestSkip 'Current lvl2vhir scripts are Python 2-era and python2 is not on PATH.'
}
elseif ($IncludeOptional) {
    Invoke-TestStep 'python2 can import the level conversion entry point' {
        $env:USAGI_DIR = $RepoRoot
        & python2 -c "import sys; sys.path.insert(0, r'$RepoRoot/Tools/python/lvl2vhir'); import lvl2vhir"
        if ($LASTEXITCODE -ne 0) {
            throw "python2 import failed with exit code $LASTEXITCODE"
        }
    }
}
else {
    Add-TestSkip 'Python 2 import smoke is opt-in; pass -IncludeOptional.'
}

Complete-TestHarness
