param(
    [string]$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..\..").Path,
    [string]$RubyPbRoot
)

$ErrorActionPreference = 'Stop'

$ResolvedRepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$RepoRoot = $ResolvedRepoRoot
$RepoParent = Split-Path -Parent $RepoRoot
$ToolsRoot = Join-Path $RepoParent 'tools'
if (-not (Test-Path (Join-Path $ToolsRoot 'usagi-dev-env.ps1'))) {
    $ToolsRoot = Join-Path (Split-Path -Parent $RepoParent) 'tools'
}
$ToolsRoot = [System.IO.Path]::GetFullPath($ToolsRoot)
. (Join-Path $ToolsRoot 'usagi-dev-env.ps1')

$RepoRoot = $ResolvedRepoRoot
$env:USAGI_DIR = $RepoRoot
$env:USAGI_RUBY = (Get-Command ruby).Source
$BuildRoot = Join-Path $ToolsRoot 'test-build\AudioToolIntegration'
$ProjectRoot = Join-Path $BuildRoot 'Project'
$OutRoot = Join-Path $BuildRoot 'out'
$AudioDir = Join-Path $ProjectRoot 'Data\VPB\Audio'
$AudioAssetDir = Join-Path $ProjectRoot 'Data\Audio'
$DefaultsDir = Join-Path $ProjectRoot 'Data\Components'
$ProtoDepsDir = Join-Path $ProjectRoot '_build\proto'

if ([string]::IsNullOrWhiteSpace($RubyPbRoot)) {
    $RubyPbRoot = Join-Path $RepoRoot '_build\ruby'
}
$RubyPbRoot = [System.IO.Path]::GetFullPath($RubyPbRoot)
if (-not (Test-Path (Join-Path $RubyPbRoot 'Engine\Audio\AudioBank.pb.rb'))) {
    throw "AudioBank Ruby protobuf was not found under $RubyPbRoot. Run project generation first or pass -RubyPbRoot."
}
$RubyPbRootForRuby = $RubyPbRoot -replace '\\', '/'

Remove-Item -LiteralPath $BuildRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $AudioDir, $AudioAssetDir, $DefaultsDir, $ProtoDepsDir, $OutRoot | Out-Null
New-Item -ItemType Junction -Path (Join-Path $ProjectRoot 'Usagi') -Target $RepoRoot | Out-Null
New-Item -ItemType Junction -Path (Join-Path $ProjectRoot '_build\ruby') -Target $RubyPbRoot | Out-Null
'{}' | Set-Content -Encoding ASCII -NoNewline -LiteralPath (Join-Path $DefaultsDir 'Defaults.yml')
'' | Set-Content -Encoding ASCII -NoNewline -LiteralPath (Join-Path $ProtoDepsDir 'deps.txt')

function Write-TestWave {
    param([string]$Path)

    $stream = [System.IO.MemoryStream]::new()
    $writer = [System.IO.BinaryWriter]::new($stream)
    try {
        $writer.Write([Text.Encoding]::ASCII.GetBytes('RIFF'))
        $writer.Write([uint32]0)
        $writer.Write([Text.Encoding]::ASCII.GetBytes('WAVE'))
        $writer.Write([Text.Encoding]::ASCII.GetBytes('fmt '))
        $writer.Write([uint32]16)
        $writer.Write([uint16]1)
        $writer.Write([uint16]1)
        $writer.Write([uint32]48000)
        $writer.Write([uint32]96000)
        $writer.Write([uint16]2)
        $writer.Write([uint16]16)
        $writer.Write([Text.Encoding]::ASCII.GetBytes('smpl'))
        $writer.Write([uint32]60)
        foreach ($Value in @(0, 0, 0, 0, 0, 0, 0, 1, 0)) {
            $writer.Write([uint32]$Value)
        }
        foreach ($Value in @(0, 0, 2, 5, 0, 0)) {
            $writer.Write([uint32]$Value)
        }
        $writer.Write([Text.Encoding]::ASCII.GetBytes('data'))
        $writer.Write([uint32]16)
        $writer.Write((New-Object byte[] 16))
        $bytes = $stream.ToArray()
        [BitConverter]::GetBytes([uint32]($bytes.Length - 8)).CopyTo($bytes, 4)
        [System.IO.File]::WriteAllBytes($Path, $bytes)
    }
    finally {
        $writer.Dispose()
        $stream.Dispose()
    }
}

$InputYaml = Join-Path $OutRoot 'audio_source.yml'
@'
AudioBank:
  soundFiles:
    - enumName: TEST_SOUND
      filename: test_sound
      stream: false
      loop: true
      volume: 1.0
      minDistance: 1.0
      maxDistance: 1000.0
      eType: AUDIO_TYPE_SFX
      eFalloff: AUDIO_FALLOFF_LINEAR
      pitchRandomisation: 0.0
      dopplerFactor: 0.0
      basePitch: 1.0
      priority: 128
      crossfade: ""
      localized: false
      crc: 0
      filterCRC: 0
      effectCRCs: []
      roomNameCRC: 0
  filters: []
  reverbs: []
  rooms: []
'@ | Set-Content -Encoding ASCII -LiteralPath $InputYaml

$ToolProject = Join-Path $RepoRoot 'Tools\Source\UsagiTools\src\Usagi.AudioToolCli\Usagi.AudioToolCli.csproj'
$BankYaml = Join-Path $AudioDir 'TestAudio.yml'
$FsidProto = Join-Path $OutRoot 'TestAudio.proto'
$BankVpb = Join-Path $OutRoot 'TestAudio.vpb'
$BankDepfile = Join-Path $OutRoot 'TestAudio.vpb.d'
$PakOut = Join-Path $OutRoot 'audio.pak'
$PakDepfile = Join-Path $OutRoot 'audio.pak.d'
$TempDir = Join-Path $OutRoot 'temp\'

Write-TestWave -Path (Join-Path $AudioAssetDir 'test_sound.wav')
Write-TestWave -Path (Join-Path $AudioDir 'test_sound.wav')

dotnet run --project $ToolProject --no-restore -- -i $InputYaml -o $BankYaml --normalize-yaml --project-root $ProjectRoot
if ($LASTEXITCODE -ne 0) {
    throw "Audio tool YAML normalization failed with exit code $LASTEXITCODE"
}

dotnet run --project $ToolProject --no-restore -- -i $BankYaml -o $FsidProto -e TestAudio -g _CLR_TEST_AUDIO_FSID_ -p --project-root $ProjectRoot
if ($LASTEXITCODE -ne 0) {
    throw "Audio tool FSID proto generation failed with exit code $LASTEXITCODE"
}
if ((Get-Content -Raw -LiteralPath $FsidProto) -notmatch 'TEST_SOUND\s*=') {
    throw 'Generated FSID proto did not contain TEST_SOUND.'
}

Push-Location $ProjectRoot
try {
    ruby -I $RubyPbRootForRuby (Join-Path $RepoRoot 'Tools\ruby\yml2vpb.rb') `
        -o $BankVpb `
        --MF $BankDepfile `
        "-R$RubyPbRootForRuby" `
        $BankYaml
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
if (-not (Test-Path $BankVpb) -or (Get-Item $BankVpb).Length -le 0) {
    throw "yml2vpb did not produce a non-empty VPB: $BankVpb"
}

$ValidationOutput = dotnet run --project $ToolProject --no-restore -- -i $BankYaml --validate --project-root $ProjectRoot 2>&1
if ($LASTEXITCODE -ne 0) {
    $ValidationOutput | ForEach-Object { Write-Host $_ }
    throw "Audio tool validation failed unexpectedly."
}

Push-Location $ProjectRoot
try {
    & (Join-Path $RepoRoot 'Tools\bin\PakFileGen.exe') $AudioDir ("-o" + $PakOut) ("-d" + $PakDepfile) ("-t" + $TempDir) -pwin
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
if (-not (Test-Path $PakOut) -or (Get-Item $PakOut).Length -le 0) {
    throw "PakFileGen did not produce a non-empty pak: $PakOut"
}

if (-not (Test-Path $PakDepfile)) {
    throw "PakFileGen did not produce a dependency file: $PakDepfile"
}

Write-Host "Audio tool integration passed: $BankYaml -> $BankVpb and $PakOut"
