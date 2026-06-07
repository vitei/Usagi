param()

$ErrorActionPreference = 'Stop'

$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$BuildRoot = Join-Path $RepoRoot 'Tools\test-build\EntityHierarchy'
$OutFile = Join-Path $BuildRoot 'all_features.bhier'
$Fixture = Join-Path $PSScriptRoot 'Fixtures\AllFeatures.yml'
$IncludeRoot = Join-Path $PSScriptRoot 'Fixtures\Includes'
$RubyStubRoot = Join-Path $PSScriptRoot 'RubyStubs'
$DepsFile = Join-Path $RepoRoot '_build\proto\deps.txt'

Remove-Item -LiteralPath $BuildRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $BuildRoot | Out-Null
New-Item -ItemType Directory -Path (Split-Path -Parent $DepsFile) -Force | Out-Null
if (-not (Test-Path $DepsFile)) {
    New-Item -ItemType File -Path $DepsFile | Out-Null
}

$previousUsagiDir = $env:USAGI_DIR
$env:USAGI_DIR = $RepoRoot

try {
    Push-Location $RepoRoot
    try {
        & ruby -I $RubyStubRoot Tools/ruby/process_hierarchy.rb -I $IncludeRoot -o $OutFile -g $Fixture
    }
    finally {
        Pop-Location
    }

    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (-not (Test-Path $OutFile)) {
        throw "process_hierarchy.rb did not produce the expected output: $OutFile"
    }
    if ((Get-Item $OutFile).Length -le 0) {
        throw "process_hierarchy.rb produced an empty hierarchy output: $OutFile"
    }

    $bytes = [System.IO.File]::ReadAllBytes($OutFile)
    $text = [System.Text.Encoding]::GetEncoding('ISO-8859-1').GetString($bytes)

    $expectedMarkers = @(
        'Usg::HierarchyHeader{entityCount=1}',
        'Usg::EntityHeader{childEntityCount=1,componentCount=4,initializerEventCount=1}',
        'Usg::Components::Identifier{name="RootValidation"}',
        'Processor::Merge{entityWithID="ChildTarget"}',
        'Usg::Events::IncreaseHealthEvent{amount=2.5}',
        'Usg::EntityHeader{childEntityCount=0,componentCount=3,initializerEventCount=0}',
        'Usg::Components::Identifier{name="ChildTarget"}',
        'Usg::Components::HealthComponent{fLife=25.0,iKillerNUID=0,uKillerTeam=0}'
    )

    foreach ($marker in $expectedMarkers) {
        if (-not $text.Contains($marker)) {
            throw "Converted hierarchy output is missing expected marker: $marker"
        }
    }

    Write-Host "Entity hierarchy validation passed: $OutFile"
}
finally {
    if ($null -eq $previousUsagiDir) {
        Remove-Item Env:\USAGI_DIR -ErrorAction SilentlyContinue
    }
    else {
        $env:USAGI_DIR = $previousUsagiDir
    }
}
