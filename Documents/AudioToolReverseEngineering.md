# Audio Tool Reverse Engineering Track

This track defines the bounded reverse-engineering pass for the shipped
binary-only audio tools under `Tools/AudioTool`. The goal is to recover the
data contracts and generated-output behavior needed to replace the tools, not
to clone the old ATF/WinForms UI.

## Scope

Read-only inputs:

- `Tools/AudioTool/AudioTool.exe`
- `Tools/AudioTool/FSIDBuilder.exe`
- `Tools/AudioTool/AudioInterop.dll`
- `Tools/AudioTool/Vitei.FMODInterop.dll`
- `Tools/AudioTool/*.config`
- `Tools/AudioTool/*.dll`
- `Tools/AudioTool/README.ja.txt`
- `Engine/Audio/**`
- `Tools/build/game_common_rake.rb`
- `Tools/build/build_config.rb`

Primary outputs:

- Confirmed `FSIDBuilder.exe` CLI behavior.
- Managed assembly inventory for public and internal audio data types.
- Audio bank field map from legacy types to `Engine/Audio/AudioBank.proto`.
- CRC and enum-generation compatibility notes.
- WAV metadata and loop-region behavior notes.
- Replacement-tool requirements that are proven by binary behavior rather than
  inference.

Non-goals:

- Rebuilding `AudioTool.exe` from missing source.
- Reproducing the legacy docking UI.
- Reintroducing ATF as the replacement tool framework.
- Decompiling third-party libraries beyond metadata needed to identify public
  dependency boundaries.

## Work Packages

### 1. Binary Inventory

List every executable, managed assembly, native DLL, config file, data file,
and README in `Tools/AudioTool`. Capture:

- File name, size, timestamp, product/file version, and architecture.
- Managed target framework for .NET assemblies.
- Strong names and assembly references.
- Native imports for FMOD, MediaInfo, SharpDX, and platform APIs.
- Embedded resources that look like schemas, icons, defaults, or serialized
  templates.

Expected deliverable: a table in the RE report plus a dependency graph showing
which binaries are tool-runtime dependencies and which are editor-only.

### 2. Black-Box Smoke

Run `FSIDBuilder.exe` and `AudioTool.exe` in a controlled working directory.
Capture:

- `--help` and invalid-argument output.
- Exit codes for missing input, malformed YAML, unknown options, and valid
  synthetic banks.
- Current-directory assumptions.
- Config-file reads and missing-DLL behavior.
- Any log files or generated temp files.

`AudioTool.exe` smoke should stay minimal: process launch, startup failure
diagnostics, config reads, and file-open behavior if it can be reached without
manual UI work. The CLI builder is the high-value target.

### 3. Managed Metadata Inspection

Inspect managed assemblies with metadata-first tooling. Preferred order:

1. .NET reflection or `ildasm` for assembly references, type names, members,
   attributes, resources, and entry points.
2. ILSpy/dotPeek or equivalent if available under the local tools directory.
3. Focused IL reading only where metadata is insufficient, especially CRC,
   enum escaping, YAML normalization, and protobuf output.

Recover type names and member lists for:

- Audio bank/editor document models.
- Sound, filter, effect, room, category, and project settings models.
- YAML load/save adapters.
- FSID/protobuf/header writers.
- CRC helpers and identifier normalization.
- WAV/media metadata readers.
- FMOD preview or import adapters.

Expected deliverable: a type/member inventory with notes on which members map
directly to the open runtime schema.

### 4. Data Contract Mapping

Map recovered fields to the runtime and build contracts:

- `Engine/Audio/AudioBank.proto`
- `Engine/Audio/AudioComponents.proto`
- `Engine/Audio/AudioEvents.proto`
- `Engine/Audio/AudioEnums.proto`
- `Engine/Audio/SoundFile.*`
- `Engine/Audio/_xaudio2/WaveFile.*`
- `Tools/Source/PakFileGen/FileFactory.cpp`
- `Tools/build/game_common_rake.rb`

For each field, classify it as:

- Required by current runtime.
- Used only by tools/build.
- Preserved but ignored by open runtime.
- FMOD/private-platform suspect.
- Derived metadata that should not be hand-authored.

Expected deliverable: a compatibility table that can drive
`Usagi.ToolCore.Audio`.

### 5. Golden Output Capture

Create tiny synthetic audio bank fixtures and run the legacy builder to capture:

- Generated FSID proto output.
- Generated C++ header output, if still supported.
- CRC values for representative names.
- Enum escaping for spaces, punctuation, digits, lowercase names, and repeated
  names.
- Behavior for missing optional fields and default values.

Do not replace `FSIDBuilder.exe` until the new builder matches these fixtures
or differences are explicitly accepted.

Expected deliverable: fixtures and expected outputs under a focused test path,
or a documented blocker if the binary cannot run headlessly.

### 6. Replacement Spec Update

Fold proven findings back into `Documents/AudioToolReplacement.md`:

- Exact CRC algorithm and normalization.
- Exact FSID proto/header formatting.
- YAML shape and default-value behavior.
- WAV metadata requirements.
- FMOD dependency decision.
- CLI compatibility requirements for switching `game_common_rake.rb`.

Expected deliverable: an implementation-ready audio-tool spec with open
questions reduced to project-data gaps rather than binary-tool unknowns.

## Subagent Brief

Use a dedicated worker in its own worktree. The worker owns only:

- `Documents/AudioToolReverseEngineering.md`
- `Documents/AudioToolReplacement.md`
- `Tools/Tests/AudioToolReverseEngineering/**`, if fixtures are added
- local tooling under the workspace tools directory, if metadata tools are installed

The worker must treat `Tools/AudioTool/**` as read-only. It should not edit
runtime audio code unless a separate implementation task is opened.

Recommended first command sequence:

```powershell
Set-Location <repo>
Get-ChildItem .\Tools\AudioTool -Force
.\Tools\AudioTool\FSIDBuilder.exe --help
```

Then install any required inspection tools under the workspace tools directory, not
inside the repository.

## Acceptance Criteria

- The binary package inventory is complete.
- `FSIDBuilder.exe` CLI behavior is captured with command lines and exit codes.
- Managed audio model types are inventoried at the level needed to implement a
  replacement.
- Legacy fields are mapped to current `Engine/Audio` protobuf/runtime fields.
- At least one golden-output fixture exists, or the reason it cannot be created
  is documented.
- `Documents/AudioToolReplacement.md` is updated with any proven facts from the
  RE pass.
- The next implementation task for `Usagi.ToolCore.Audio` can proceed without
  guessing at builder output behavior.

## Worker A Findings - Binary Inventory And Smoke

Captured on 2026-05-13 from a dedicated `codex/audio-tool-re` worktree.
`Tools/AudioTool/**` was treated as read-only. Scratch captures were written
outside the repository.

### Complete File Inventory

All files had `LastWriteTimeUtc` `2026-05-13 11:27:05`. Managed/native
classification was determined from PE metadata where practical; file versions
come from Windows version resources. The shipped `.config` files target
`.NETFramework,Version=v4.5.2` for the two executables.

| File | Size | Kind | Arch | File version |
|---|---:|---|---|---|
| `Atf.Core.dll` | 322560 | managed assembly | AnyCPU | 1.0.0.0 |
| `Atf.Gui.dll` | 1076224 | managed assembly | AnyCPU | 1.0.0.0 |
| `Atf.Gui.OpenGL.dll` | 100352 | managed assembly | AnyCPU | 1.0.0.0 |
| `Atf.Gui.WinForms.dll` | 2029568 | managed assembly | AnyCPU | 1.0.0.0 |
| `Atf.IronPython.dll` | 8192 | managed assembly | AnyCPU | 1.0.0.0 |
| `AudioInterop.dll` | 75776 | managed assembly | AnyCPU | 1.0.0.0 |
| `AudioInterop.dll.config` | 415 | config | n/a | n/a |
| `AudioTool.exe` | 32768 | managed executable | AnyCPU | 1.0.0.0 |
| `AudioTool.exe.config` | 184 | config | n/a | n/a |
| `AudioTool.vshost.exe` | 22696 | managed executable | AnyCPU | 14.0.23107.0 |
| `AudioTool.vshost.exe.config` | 184 | config | n/a | n/a |
| `AudioTool.vshost.exe.manifest` | 479 | manifest | n/a | n/a |
| `Bespoke.Common.dll` | 20480 | managed assembly | AnyCPU | 5.0.0.0 |
| `Bespoke.Common.Osc.dll` | 22528 | managed assembly | AnyCPU | 0.0.0.0 |
| `DDSUtils.dll` | 475136 | managed/mixed-mode PE | AnyCPU/CLR | n/a |
| `fmod.dll` | 1516544 | native PE | x86 | 1.10.0 (build 90329) |
| `fmod64.dll` | 1750016 | native PE | x64 | 1.10.0 (build 90329) |
| `fmodL.dll` | 1694720 | native PE | x86 | 1.10.0 (build 90329) |
| `fmodL64.dll` | 1953280 | native PE | x64 | 1.10.0 (build 90329) |
| `FSIDBuilder.exe` | 26624 | managed executable | AnyCPU | 1.0.0.0 |
| `FSIDBuilder.exe.config` | 184 | config | n/a | n/a |
| `FSIDBuilder.vshost.exe` | 22696 | managed executable | AnyCPU | 14.0.23107.0 |
| `FSIDBuilder.vshost.exe.config` | 184 | config | n/a | n/a |
| `FSIDBuilder.vshost.exe.manifest` | 479 | manifest | n/a | n/a |
| `IronPython.dll` | 1798656 | managed assembly | AnyCPU | 2.7.3.1000 |
| `IronPython.xml` | 400499 | XML docs | n/a | n/a |
| `ja/Atf.Gui.WinForms.resources.dll` | 23040 | managed satellite assembly | AnyCPU | 1.0.0.0 |
| `libcrashreport_net.dll` | 32768 | managed assembly | AnyCPU | 1.1.4444.20746 |
| `MediaInfo.dll` | 4721488 | native PE | x86 | 0.7.81.0 |
| `MediaInfoDotNet.dll` | 31744 | managed assembly | AnyCPU | 0.7.4694.29125 |
| `MediaInfoDotNet.xml` | 37800 | XML docs | n/a | n/a |
| `Microsoft.Dynamic.dll` | 1044480 | managed assembly | AnyCPU | 1.1.0.21 |
| `Microsoft.Dynamic.xml` | 362156 | XML docs | n/a | n/a |
| `Microsoft.Scripting.dll` | 143872 | managed assembly | AnyCPU | 1.1.0.21 |
| `Microsoft.Scripting.xml` | 201897 | XML docs | n/a | n/a |
| `NDesk.Options.dll` | 22016 | managed assembly | AnyCPU | 0.2.1.0 |
| `Nito.KitchenSink.CRC.dll` | 10752 | managed assembly | AnyCPU | 1.1.0.0 |
| `Nito.KitchenSink.CRC.xml` | 21893 | XML docs | n/a | n/a |
| `protobuf-net.dll` | 197632 | managed assembly | AnyCPU | 2.0.0.668 |
| `protobuf-net.xml` | 159532 | XML docs | n/a | n/a |
| `README.ja.txt` | 390 | Shift-JIS text/readme | n/a | n/a |
| `SharpDX.Direct2D1.dll` | 426496 | managed assembly | AnyCPU | 2.6.3 |
| `SharpDX.dll` | 567808 | managed assembly | AnyCPU | 2.6.3 |
| `SharpDX.DXGI.dll` | 116224 | managed assembly | AnyCPU | 2.6.3 |
| `Tao.Cg.dll` | 61440 | managed assembly | AnyCPU | 1.4.1.1 |
| `Tao.DevIl.dll` | 40960 | managed assembly | AnyCPU | 1.6.8.2 |
| `Tao.OpenGl.dll` | 1175552 | managed assembly | AnyCPU | 2.1.0.7 |
| `Tao.Platform.Windows.dll` | 57344 | managed assembly | AnyCPU | n/a |
| `Vitei.ATFExtensions.dll` | 37888 | managed assembly | AnyCPU | 1.0.0.0 |
| `Vitei.FMODInterop.dll` | 99328 | managed assembly | AnyCPU | 1.0.0.0 |
| `WeifenLuo.WinFormsUI.Docking.dll` | 426496 | managed assembly | AnyCPU | 2.3.2.0 |
| `YamlDotNet.dll` | 136704 | managed assembly | AnyCPU | 0.0.0 |
| `YamlDotNet.xml` | 192968 | XML docs | n/a | n/a |

Managed dependency highlights:

- `FSIDBuilder.exe` uses `AudioInterop.dll`, `NDesk.Options.dll`, and
  framework assemblies.
- `AudioTool.exe` uses ATF WinForms shell assemblies, `AudioInterop.dll`,
  `Vitei.ATFExtensions.dll`, `WeifenLuo.WinFormsUI.Docking.dll`, and framework
  WinForms assemblies.
- `AudioInterop.dll` references `Atf.Core`, `Atf.Gui`, `Atf.Gui.WinForms`,
  `MediaInfoDotNet`, `Nito.KitchenSink.CRC`, `Vitei.FMODInterop`,
  `YamlDotNet`, and framework WinForms/design assemblies.
- `Vitei.FMODInterop.dll` is managed and references `Atf.Core`,
  `System.ComponentModel.Composition`, and `mscorlib`; native FMOD is supplied
  separately by the `fmod*.dll` files.
- `AudioInterop.dll.config` contains a binding redirect for `Tao.OpenGl`
  `0.0.0.0-2.1.0.7` to `2.1.0.7`.

Native dependency highlights from `dumpbin /dependents`:

- `fmod.dll`, `fmod64.dll`, `fmodL.dll`, and `fmodL64.dll` import
  `WS2_32.dll`, `WINMM.dll`, `MSACM32.dll`, `KERNEL32.dll`, `USER32.dll`,
  `ADVAPI32.dll`, and `ole32.dll`.
- `MediaInfo.dll` imports `KERNEL32.dll`.
- `DDSUtils.dll` imports `MSVCR80.dll`, `KERNEL32.dll`, `USER32.dll`,
  `msvcm80.dll`, `MSVCP80.dll`, and `mscoree.dll`.
- `libcrashreport_net.dll` imports `mscoree.dll`.

### `FSIDBuilder.exe` Smoke Results

All commands were run from a scratch directory with the tool path pointing at
`Tools\AudioTool\FSIDBuilder.exe`.

| Case | Command | Exit code | Result |
|---|---|---:|---|
| Help | `FSIDBuilder.exe --help` | 0 | Printed usage and options to stdout. |
| No args | `FSIDBuilder.exe` | -532462766 | Unhandled `System.IO.FileNotFoundException: No input file specified.` |
| Unknown option | `FSIDBuilder.exe --bogus` | -532462766 | Unknown option was ignored, then failed as no input file. |
| Missing input | `FSIDBuilder.exe -i does-not-exist.yml -o missing.proto -p` | -532462766 | Unhandled `System.IO.FileNotFoundException: Input file not found.` |
| Missing output | `FSIDBuilder.exe -i minimal-legacy.yml -p` | -532462766 | Unhandled `System.ArgumentException: The path is not of a legal form.` |
| Malformed YAML | `FSIDBuilder.exe -i malformed.yml -o malformed.proto -p` | -532462766 | Unhandled `YamlDotNet.Core.YamlException` from `AudioBankDocument.LoadAudioBank`. |
| Valid proto | `FSIDBuilder.exe -i minimal-legacy.yml -o minimal.proto -e TestAudio -g _CLR_TEST_AUDIO_FSID_ -p` | 0 | Generated protobuf enum output; no stdout/stderr. |
| Valid header | `FSIDBuilder.exe -i minimal-legacy.yml -o minimal.h -e TestAudio -g _CLR_TEST_AUDIO_FSID_` | 0 | Generated C++ header output; no stdout/stderr. |

`--help` output:

```text
Usage: fsidbuilder [OPTIONS]
Build a .fsid file from an AudioBank .yml file.

Options:
  -i, --input=VALUE          audio bank (.yml) to load.
  -o, --output=VALUE         output file to generate.
  -e, --enumName=VALUE       name of enum count constant to use
  -g, --ifndefName=VALUE     name of #ifndef guard to use
  -p, --proto                generate a protocol buffer file instead of a C++
                               header
  -h, --help                 show this message and exit
```

Synthetic valid input findings:

- The legacy YAML model for `AudioInterop.dll` deserializes
  `Vitei.AudioInterop.AudioBankYaml` with `AudioBank.soundFiles` only.
- `SoundFileOut` contains `enumName`, `filename`, `stream`, `loop`, `volume`,
  `minDistance`, `maxDistance`, `eType`, `eFalloff`, `pitchRandomisation`,
  `priority`, `crossfade`, `basePitch`, `dopplerFactor`, `localized`, and
  `crc`.
- `eType` and `eFalloff` must be integer values in legacy YAML. Supplying
  symbolic proto names such as `AUDIO_TYPE_SFX` causes a YAML
  `System.FormatException`.
- Runtime-era fields `filterCRC`, `effectCRCs`, `roomNameCRC`, `filters`,
  `reverbs`, and `rooms` are not part of the recovered legacy
  `AudioInterop.AudioBank` metadata observed in this smoke pass.
- Input order is preserved in generated enum output. A two-entry bank with
  `Z_LAST` before `A_FIRST` generated `Z_LAST` first.
- CRCs are generated from `enumName`, not from the source YAML `crc` value. The
  fixture input sets `crc: 0`; generated `LASER_SHOT` is `615283986`.
- Proto enum values are written as signed decimal integers. If the generated
  32-bit CRC is above `Int32.MaxValue`, proto output can be negative; header
  output uses `static const unsigned int`.

The reproducible fixture is under
`Tools/Tests/AudioToolReverseEngineering/FSIDBuilderSmoke`. `Run.ps1`
generates both proto and header output with the legacy builder, normalizes the
timestamp and copyright character in the generated proto banner, and compares
against expected proto/header files. Coverage includes:

- Basic one-sound output.
- Input order preservation.
- Leading/trailing whitespace trim.
- Literal space replacement with underscore.
- Lowercase name preservation.
- Signed protobuf output for CRCs above `Int32.MaxValue`.

### `AudioTool.exe` Smoke Results

Commands were bounded to process launch and dependency behavior only.

| Case | Command | Exit code | Result |
|---|---|---:|---|
| Original launch from scratch cwd | `Start-Process AudioTool.exe` from a scratch `audiotool` directory | killed after 5 seconds | Process did not exit within five seconds, produced no stdout/stderr, and created no files in the scratch cwd. This is consistent with a GUI startup path. |
| Isolated launch missing dependencies | copy only `AudioTool.exe` and `AudioTool.exe.config`, then launch from isolated scratch dir | -532462766 | Unhandled `System.IO.FileNotFoundException`: could not load `Atf.Core, Version=1.0.0.0`. |

`AudioTool.exe.config` and `FSIDBuilder.exe.config` only specify
`.NETFramework,Version=v4.5.2` under `<supportedRuntime>`. No custom config
reads were observed during the bounded smoke.

## Managed Metadata And IL Findings

Inspection used PowerShell reflection, Windows SDK `ildasm`, and `ilspycmd`.
ILSpy project output was written outside the repository.

All primary audio binaries are .NET Framework 4.5.2 managed assemblies:

| Assembly | Version | Entry point | Role |
|---|---:|---|---|
| `AudioInterop.dll` | 1.0.0.0 | none | recoverable model, YAML, metadata, FSID/PB writer |
| `FSIDBuilder.exe` | 1.0.0.0 | `FSIDBuilder.Program.Main(string[] args)` | console builder |
| `AudioTool.exe` | 1.1.0.3 | `Vitei.AudioTool.Program.Main()` | ATF/WinForms editor shell |
| `Vitei.FMODInterop.dll` | 1.0.0.0 | none | FMOD wrapper and preview service |

Important assembly references:

- `FSIDBuilder.exe`: `NDesk.Options`, `AudioInterop`.
- `AudioTool.exe`: `Atf.Core`, `Atf.Gui`, `Atf.Gui.WinForms`,
  `Vitei.ATFExtensions`, `AudioInterop`, `Vitei.FMODInterop`,
  `System.ComponentModel.Composition`.
- `AudioInterop.dll`: `Atf.Core`, `Atf.Gui`, `Atf.Gui.WinForms`,
  `MediaInfoDotNet`, `Nito.KitchenSink.CRC`, `YamlDotNet`,
  `Vitei.FMODInterop`, WinForms/design framework assemblies.
- `Vitei.FMODInterop.dll`: `Atf.Core`,
  `System.ComponentModel.Composition`, framework assemblies.

Recovered core model types:

- `Vitei.AudioInterop.AudioBankYaml`
  - `AudioBank AudioBank { get; set; }`
- `Vitei.AudioInterop.AudioBank`
  - `List<SoundFileOut> soundFiles { get; set; }`
- `Vitei.AudioInterop.SoundFileOut`
  - `enumName`, `filename`, `stream`, `loop`, `volume`, `minDistance`,
    `maxDistance`, `eType`, `eFalloff`, `pitchRandomisation`, `priority`,
    `crossfade`, `basePitch`, `dopplerFactor`, `localized`, `crc`
  - defaults: `volume=1`, `minDistance=1`, `maxDistance=1000`, `eType=1`,
    `priority=128`, `basePitch=1`
- `Vitei.AudioInterop.DomNodeAdapters.SoundFileDefinition`
  - editor fields for name, file, stream, loop, volume, distance, type,
    falloff, channel count, sample rate, bit depth, bit rate, duration,
    priority, random pitch, base pitch, doppler, crossfade, and localization
  - FMOD preview handles: `FMOD.Sound Sound`, `FMOD.Channel Channel`
- `Vitei.AudioInterop.DomNodeAdapters.AudioBankDefinition`
  - `IList<SoundFileDefinition> Files`

Visible enum helpers:

- `AudioType.InternalEnum`: `AUDIO_TYPE_MUSIC = 0`,
  `AUDIO_TYPE_SFX = 1`, `AUDIO_TYPE_UI = 2`
- `AudioFalloff.InternalEnum`: `AUDIO_FALLOFF_LINEAR = 0`,
  `AUDIO_FALLOFF_LOGARITHMIC = 1`
- `ChannelCount.InternalEnum`: `CHANNEL_COUNT_MISSING = 0`,
  `CHANNEL_COUNT_MONO = 1`, `CHANNEL_COUNT_STEREO = 2`

The legacy managed model does not expose the newer/current runtime fields
`filterCRC`, `effectCRCs`, `roomNameCRC`, `filters`, `reverbs`, or `rooms`.
The replacement should still preserve and edit those fields because the open
runtime schema includes them.

### CRC And FSID Generation

`FSIDBuilder.Program.Main` parses options and delegates to
`AudioBankDocument`:

- `--proto`: sets `AudioBankDocument.UsagiEnumName`, then calls `WritePB`.
- default/header path: calls `WriteFSID`.

CRC generation is:

```csharp
string text = soundFileDefinition.Name.Trim().Replace(' ', '_');
CRC32 crc = new CRC32();
crc.Initialize();
uint value = BitConverter.ToUInt32(crc.ComputeHash(Encoding.UTF8.GetBytes(text)), 0);
```

`Nito.KitchenSink.CRC.CRC32.Definition.Default` is standard reflected
IEEE/PKZIP CRC-32:

- polynomial `0x04C11DB7`
- initializer `0xFFFFFFFF`
- final xor `0xFFFFFFFF`
- reflected input bytes
- reflected result before final xor
- little-endian hash bytes

Enum normalization is intentionally narrow:

- `Trim()`
- replace literal space `' '` with underscore `_`
- preserve case
- preserve YAML input order
- do not sanitize tabs, punctuation, leading digits, or duplicates

`WriteFSID` emits:

```text
#ifndef <ifndef>
#define <ifndef>

// Generated by Usagi Audio Tool.
static const unsigned int <enumName> = <crc>;
...
static const unsigned int <enumName>_COUNT = <count>;

#endif
```

`WritePB` emits:

- a time-variable banner
- `import 'nanopb.proto';`
- enum type name `UsagiEnumName.Trim().Replace(" ", "")`
- `option (nanopb_enumopt).long_names = false;`
- `option (nanopb_enumopt).lua_generate = true;`
- enum entries as `<normalizedName> = <(int)crc>;`
- no count entry

The signed cast matters: CRCs above `0x7fffffff` become negative `.proto`
enum values. Header output remains unsigned.

### YAML And Metadata Behavior

`AudioBankDocument.LoadAudioBank` uses YamlDotNet to read `AudioBankYaml` and
expects a top-level `AudioBank:` mapping with `soundFiles`. It only replaces
ERB-style `AudioType` expressions before deserialization:

```text
<%= AudioType::AUDIO_TYPE_SFX %>
```

`AudioFalloff` is not converted by that helper. `WriteAudioBank` emits
defaults and only escapes `eType` back to an `AudioType` ERB token, so `eFalloff`
stays numeric in legacy output.

`Vitei.AudioInterop.WavMetadata` reads:

- RIFF/WAVE header fields with `BinaryReader`
- `channels`, `sampleRate`, `fmtAvgBPS`, `bitDepth`
- `bitRate = fmtAvgBPS * 8`
- duration through `MediaInfoDotNet.MediaFile.duration`
- `smpl` chunks through `GTamir.RiffParser`

Recovered `smpl` loop fields are `ID`, `Type`, `Start`, `End`, `Fraction`, and
`PlayCount`; loop type values include forward `0`, alternating `1`, and
backward `2`.

`Vitei.FMODInterop.SoundSystem` exposes editor-preview style operations:
`Initialize`, `Teardown`, `StopAll`, `GetChannelGroup`, `LoadSound`,
`PlaySound`, and `ForceUpdate`. No FMOD bank/event authoring data model was
visible.

## Runtime And Build Contract Mapping

`Audio::LoadSoundArchive()` reads `AudioBank` VPB files and iterates
`soundFiles`, `filters`, and `reverbs`. It does not load `AudioBank.rooms` into
runtime room state.

Runtime-critical fields:

- `SoundFileDef`: `filename`, `localized`, `loop`, `volume`, `minDistance`,
  `maxDistance`, `eType`, `eFalloff`, `pitchRandomisation`, `basePitch`,
  `dopplerFactor`, `priority`, `crc`, `filterCRC`, `effectCRCs`
- `AudioFilterDef`: `crc`, `enumName`, `eFilter`, `fFrequency`, `fOneOverQ`
- `ReverbEffectDef`: `effectDef.crc`, `effectDef.eEffectType`, `enumName`,
  `crc`, `decayTime`, `density`, `reflectionsDelay`, `reflectionsGain`,
  `reverbDelay`, `reverbGain`, `roomFilterFreq`, `roomFilterHF`,
  `roomFilterMain`, `roomSize`, `wetDryMix`

Build-critical fields:

- `soundFiles[].enumName`: FSID enum/proto generation
- `soundFiles[].crc`: generated IDs and runtime lookup
- `soundFiles[].filename`: VPB/runtime load path and pak dependency discovery
- `soundFiles[].stream`: pak dependency tag, not current XAudio streaming
- schema field names/nesting under `AudioBank`: `yml2vpb.rb` compatibility

Preserved but ignored or weakly used by open runtime:

- `soundFiles[].stream` for playback
- `crossfade`
- `roomNameCRC`
- `AudioBank.rooms`
- `AudioRoomDef.*`
- reverb/effect data is constructed, but `Audio_ps::EnableEffect()` and
  `DisableEffect()` are stubs in the XAudio backend

Build flow:

- `build_audio()` scans `Data/VPB/Audio/*.yml`.
- It runs:

```text
FSIDBuilder.exe --proto -i="Data/VPB/Audio/<bank>.yml" \
  -o=<Project>/audio_gen/<bank>.proto \
  -e=<bank>Audio -g=_CLR_<BANK>_FSID_
```

- `yml2vpb.rb` converts all `Data/VPB/**/*.yml`, including audio banks, to
  `_romfiles/<platform>/VPB/**/*.vpb`.
- The PC data build copies `Data/Audio/*.wav` to `_romfiles/win/Audio`.

PakFileGen audio dependency discovery is different from the PC copy path. It
identifies YAML under an `Audio` directory as audio YAML, runs `yml2vpb.rb`,
then reads `AudioBank.soundFiles[].filename` and expects each WAV beside the
YAML file:

```text
<directory containing audio YAML>/<filename>.wav
```

It tags dependencies as `stream` when `stream: true`, otherwise `loaded`.

## Consolidated Next Implementation Requirements

The next `Usagi.ToolCore.Audio` slice can proceed with these requirements:

1. Implement an `AudioBank` YAML model that preserves all current runtime
   schema fields, even though the legacy managed tool only modeled
   `soundFiles`.
2. Implement an `FSIDBuilder.exe`-compatible CLI with `--proto`, `-i`, `-o`,
   `-e`, and `-g`.
3. Generate CRCs using standard reflected IEEE/PKZIP CRC-32 over
   `enumName.Trim().Replace(' ', '_')`.
4. Preserve legacy output order and signed protobuf enum values.
5. Generate C++ headers only as a compatibility path; project builds currently
   use `--proto`.
6. Validate enum names more strictly than the legacy builder, because legacy
   output can silently become invalid for punctuation, leading digits, tabs, or
   duplicates.
7. Parse WAV metadata for editor diagnostics from `fmt `, `data`, and `smpl`;
   do not make derived metadata authoritative source.
8. Add a build integration test covering YAML -> FSID proto -> `yml2vpb.rb` VPB.
9. Add a PakFileGen compatibility test or explicitly fix the WAV source-path
   mismatch between `Data/Audio` and audio YAML-adjacent pak discovery.

## Remaining Gaps

- No checked-in project audio bank exists under `Data/VPB/Audio`, so parity is
  currently limited to synthetic YAML fixtures.
- Punctuation, leading digits, tabs, and duplicate enum names are intentionally
  not normalized by the legacy tool. The replacement should validate/reject
  them rather than trying to preserve broken output.
- `AudioTool.exe` was not manually exercised beyond startup. File-open and
  settings behavior should be tested only if needed for replacement
  requirements.
- Historical project use of `filterCRC`, `effectCRCs`, `roomNameCRC`, and rooms
  is still unknown because those fields are not represented in the legacy
  managed tool model.
