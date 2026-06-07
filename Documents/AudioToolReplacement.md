# Audio Tool Replacement

This document defines the replacement scope for the binary-only audio tooling in
`Tools/AudioTool`. It is based on the runtime contract in `Engine/Audio`, the
audio protobufs, the project build rules, `Tools/Source/PakFileGen`, and the
remaining shipped binaries. The focused binary-analysis work is tracked in
[AudioToolReverseEngineering.md](AudioToolReverseEngineering.md).

## Current Runtime Contract

Audio content reaches the engine as an `AudioBank` protocol buffer generated
from YAML under `Data/VPB/Audio/*.yml`. `Audio::LoadSoundArchive()` reads that
VPB into `AudioBank`, creates one platform `SoundFile` per `SoundFileDef`, and
indexes each sound by `SoundFileDef.crc`. `PlaySound` and `PlayMusic` events
carry this CRC in `uAudioID`; runtime code does not look up sounds by string.

On Windows, each `SoundFileDef.filename` resolves to `Audio/<filename>.wav`, or
`Audio/<localized-subdir>/<filename>.wav` when `localized` is true and a
localized subdirectory was supplied to `LoadSoundArchive()`. The XAudio backend
loads WAV data through `WaveFileReader`, including `smpl` forward-loop metadata.
The current PC data build also copies any `Data/Audio/*.wav` directly to
`_romfiles/win/Audio`.

The bank fields that matter today are:

- Sound identity: `enumName`, `filename`, `crc`.
- Playback behavior: `stream`, `loop`, `volume`, `eType`, `priority`,
  `basePitch`, `pitchRandomisation`.
- 3D behavior: `minDistance`, `maxDistance`, `eFalloff`, `dopplerFactor`.
- Localization: `localized`.
- DSP references: `filterCRC`, `effectCRCs`.
- Declared but weakly or not used: `crossfade`, `roomNameCRC`, `AudioRoomDef`.

`stream` is important to preserve because the pak generator tags WAV
dependencies as streamed or loaded, but the open XAudio runtime currently loads
the whole WAV into memory.

Filters and effects are represented in `AudioBank` as filters and reverbs.
`SoundFile::InitInt()` resolves `filterCRC` and `effectCRCs`, and
`SoundObject_ps::Start()` applies a source voice filter if present. The
replacement should keep these fields even though the current XAudio effect path
is incomplete: `Audio_ps::EnableEffect()` and `DisableEffect()` are empty, and
rooms are loaded into the bank but not consumed by `Audio::LoadSoundArchive()`.

## Existing Binary-Only Tooling

`Tools/AudioTool` contains only binaries and dependencies for the editor and
ID builder:

- `AudioTool.exe`: legacy .NET Framework 4.5.2 editor shell.
- `FSIDBuilder.exe`: command-line generator used by `build_audio()` in
  `Tools/build/game_common_rake.rb`.
- `AudioInterop.dll`: managed library exposing the recoverable data model,
  YAML load/save, metadata reading, editor adapters, and FSID/PB writers.
- `Vitei.FMODInterop.dll`, `fmod*.dll`, `MediaInfo.dll`, ATF, SharpDX, Tao,
  YamlDotNet, protobuf-net, and supporting UI/runtime libraries.

`FSIDBuilder.exe --help` shows the supported command line:

```text
fsidbuilder [OPTIONS]
-i, --input=VALUE       audio bank (.yml) to load.
-o, --output=VALUE      output file to generate.
-e, --enumName=VALUE    name of enum count constant to use
-g, --ifndefName=VALUE  name of #ifndef guard to use
-p, --proto             generate a protocol buffer file instead of a C++ header
```

The generated project build path uses:

```text
FSIDBuilder.exe --proto -i="Data/VPB/Audio/<bank>.yml" \
  -o=<Project>/audio_gen/<bank>.proto \
  -e=<bank>Audio -g=_CLR_<BANK>_FSID_
```

Reflection over `AudioInterop.dll` shows the editor-facing sound model contains
`enumName`, `filename`, `stream`, `loop`, `volume`, `minDistance`,
`maxDistance`, `eType`, `eFalloff`, `pitchRandomisation`, `priority`,
`crossfade`, `basePitch`, `dopplerFactor`, `localized`, and `crc`. The editor
also tracked derived WAV metadata: filesize, channel count, sample rate, bit
depth, bit rate, duration, and loop state.

Focused IL inspection of the shipped binaries recovered the compatibility
boundary for generated IDs:

- `FSIDBuilder.exe` reads `AudioBank.soundFiles` only.
- The CRC input is `enumName.Trim().Replace(' ', '_')`.
- CRC generation uses the default `Nito.KitchenSink.CRC.CRC32` algorithm,
  equivalent to standard reflected IEEE/PKZIP CRC-32.
- Generated C++ headers emit unsigned CRCs as
  `static const unsigned int <name> = <crc>;`.
- Generated protobuf enums cast the CRC to signed `int`, so values above
  `0x7fffffff` are emitted as negative decimal enum values.
- Sound output preserves YAML input order.
- The legacy builder trims leading/trailing whitespace and replaces literal
  spaces with underscores, but it does not uppercase names or sanitize tabs,
  punctuation, leading digits, or duplicates.

## Data Model For The Replacement

Use an explicit source document that round-trips to the existing
`AudioBank` YAML shape:

```yaml
AudioBank:
  soundFiles:
    - enumName: LASER_SHOT
      filename: laser_shot
      stream: false
      loop: false
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
```

The authoritative editable fields should be split into:

- Stable source fields: every field serialized into `AudioBank`.
- Derived metadata: WAV size, format, duration, loop start/length, and validation
  diagnostics. These are displayed and cached if helpful but should be
  reproducible from the source WAV.
- Generated IDs: CRC values and FSID enum/proto entries. These must be
  deterministic and regenerated by the build.

The replacement must not require hand-editing numeric CRCs. It should compute
`crc` from `enumName.Trim().Replace(' ', '_')` using standard reflected
IEEE/PKZIP CRC-32, then write the unsigned 32-bit value into the `AudioBank`
output. FSID proto output must preserve the legacy signed `int` cast for enum
values, while header output must keep unsigned constants.

## Minimum Viable Editor

The first replacement should be a pragmatic editor in the existing
`Tools/Source/UsagiTools` stack rather than another opaque binary. Minimum
features:

- Open and save `Data/VPB/Audio/*.yml` preserving the existing `AudioBank`
  protobuf-compatible structure.
- Show one table row per sound with editable runtime fields and read-only WAV
  metadata.
- Browse/select `Data/Audio/*.wav`; store `filename` without the `.wav`
  extension to match runtime loading.
- Add, duplicate, remove, and rename sound entries.
- Generate or refresh `enumName` and `crc` deterministically.
- Validate enum names before generation. The legacy builder barely normalizes
  names, so the replacement should reject names that would produce invalid C++
  or protobuf identifiers instead of silently emitting broken output.
- Validate in-editor before save and from CLI during builds.
- Export the FSID proto/header output currently produced by `FSIDBuilder.exe`.

Nice-to-have follow-up features:

- WAV preview playback with loop-region preview.
- Filter/reverb/room editors once the runtime effect path is clarified.
- Localization folder preview and missing-localized-file reports.
- Batch import that derives enum names from filenames.

## Build And Runtime Implementation Plan

1. Add a managed `Usagi.ToolCore.Audio` model mirroring `AudioBank.proto` and
   the editor metadata above. Reuse YamlDotNet, which is already used by the
   modern tools project.
2. Implement a command-line audio builder with two outputs:
   `AudioBank` YAML normalization/validation and generated FSID proto/header.
   Keep CLI flags compatible with `FSIDBuilder.exe` so `game_common_rake.rb`
   can switch tools without changing project scripts.
3. Implement WAV metadata reading in managed code. Required parser support:
   RIFF/WAVE, `fmt `, `data`, and `smpl` forward loops. Match
   `WaveFileReader` behavior for loop start and loop length.
4. Port the CRC/enum generation. Preserve the legacy standard CRC-32,
   space-to-underscore normalization, input ordering, header formatting, and
   signed protobuf enum values. Verify against the captured synthetic fixtures
   and a recovered project bank before replacing `FSIDBuilder.exe` in the
   build.
5. Add editor UI to `Usagi.ToolShell` after the CLI and model are validated.
6. Update `Tools/build/build_config.rb` and `game_common_rake.rb` to call the
   new builder. Keep the old binary path available behind an escape hatch until
   parity is proven.
7. Add build tests that create a temporary bank, generate FSID output, generate
   a VPB through the normal `yml2vpb.rb` path, and confirm PakFileGen discovers
   referenced WAV files.

Current implementation status:

- `Tools/Source/UsagiTools/src/Usagi.ToolCore/Audio` contains the first managed
  audio-bank parser, normalizing writer, validator, and FSID generation
  library.
- `Tools/Source/UsagiTools/src/Usagi.AudioToolCli` provides a
  `FSIDBuilder.exe`-compatible command-line surface for `-i`, `-o`, `-e`, `-g`,
  and `--proto`, plus `--normalize-yaml`, `--validate`, and
  `--project-root`.
- `Tools/Tests/AudioToolBuilder/Run.ps1` verifies the new CLI against the
  legacy golden proto/header fixtures captured from `FSIDBuilder.exe`, and now
  checks normalized YAML and validation failure behavior.
- `Tools/Tests/AudioToolIntegration/Run.ps1` creates a synthetic external
  project, normalizes an audio bank, generates FSID proto output, converts the
  bank through `yml2vpb.rb`, and verifies `PakFileGen.exe` packages the bank and
  its referenced WAV.
- `Tools/Source/PakFileGen/FileFactory.cpp` now invokes Ruby through an
  explicit `USAGI_RUBY` override when provided, and records audio WAV
  dependencies using the same package-relative name as the loaded WAV resource.
- `Tools/build/build_config.rb`, `Tools/build/generator_util.rb`, and
  `Tools/build/game_common_rake.rb` now route audio FSID generation through the
  managed CLI by default. Set `USAGI_USE_LEGACY_AUDIO_TOOL=1` to use the shipped
  `FSIDBuilder.exe` during parity checks.
- `Tools/Tests/AudioToolBuildRules/Run.ps1` verifies the default generated
  build-rule command, the legacy escape hatch, and a synthetic FSID proto build.
- `Tools/Tests/AudioToolParity/Run.ps1` verifies direct managed-vs-legacy FSID
  parity for a project-shaped legacy-compatible bank, then records the expected
  limitation that `FSIDBuilder.exe` rejects current full-schema YAML containing
  fields such as `filterCRC`.
- `Tools/Source/UsagiTools/src/Usagi.ToolShell/AudioEditorTab.cs` adds the
  first artist-facing audio bank editor to the Avalonia tool shell. It can open
  and save audio-bank YAML, add/duplicate/remove sounds, edit sound-file runtime
  fields, browse referenced WAV files, show parsed WAV metadata, refresh CRCs,
  validate through `AudioBankValidator`, edit filters/reverbs/rooms, preview WAV
  playback on Windows, and export FSID proto/header files from the same managed
  generator used by builds.

No original game audio bank data is available in this workspace. Usagi was only
used for one shipped game, and that source/data is not present, so parity must be
proved with synthetic contract fixtures unless the original project data is
recovered later.

Remaining implementation work after switching project builds:

- Keep `USAGI_USE_LEGACY_AUDIO_TOOL=1` only as a legacy-compatible input escape
  hatch. It cannot process normalized/current full-schema audio YAML.
- Replace basic Windows WAV preview with engine-backed preview once the Usagi
  preview host exists, so attenuation, looping, filters, and runtime playback
  behavior can be tested in-editor.
- Add ergonomic reference pickers for assigning sound filter/effect/room CRCs
  from the bank's named filter, reverb, and room collections.

Implemented WAV metadata support:

- `WaveMetadataReader` parses RIFF/WAVE `fmt `, `data`, and `smpl` chunks.
- Forward `smpl` loops match `WaveFileReader` semantics, including loop length
  `end - start + 1`.
- Validation with `--project-root` now parses referenced WAV files, warns when
  a looping sound has no forward `smpl` loop, and reports parse/format issues
  as diagnostics.

The runtime does not need an audio-system rewrite for the replacement editor.
It does need defects and ambiguities captured as follow-up issues: effect enable
and disable are stubs, `AudioRoomDef` is not loaded into runtime state,
`crossfade` is present in the schema but not wired into `MusicManager`, and the
XAudio filter initialization appears to assign the filter type into the
frequency field.

## Validation Strategy

Validation should run in three layers:

- Document validation: required fields present, enum values known, names within
  nanopb max sizes, `effectCRCs` count no more than four, distances sane,
  `maxDistance` greater than or equal to `minDistance`, priority between 0 and
  255, volume non-negative, and filenames extensionless.
- Asset validation: referenced WAV exists in `Data/Audio` or the selected
  localization folder, RIFF format is parseable, looped sounds have valid loop
  metadata or explicit loop intent, and stream settings are reported.
- Compatibility validation: generated `AudioBank` VPB can be read by
  `Audio::LoadSoundArchive()`, generated FSID proto compiles through the
  project protobuf pipeline, and `Tools/Source/PakFileGen/FileFactory.cpp`
  still discovers the WAV dependencies from the YAML.

Golden tests should compare generated FSID output and CRCs against legacy
`FSIDBuilder.exe` for at least one bank before the binary is retired. If no
legacy project data is available, keep the legacy tool in CI as an oracle until
sample banks are recovered.

## FMOD Questions

The current runtime shown in this repository is WAV/XAudio-oriented; it does not
load FMOD banks at runtime. FMOD appears only in `Tools/AudioTool` through
`Vitei.FMODInterop.dll` and the `fmod*.dll` binaries, which suggests the old
editor may have used FMOD for preview, analysis, or a non-public platform path.
Before deleting the FMOD dependency, answer:

- Was FMOD preview playback required to match shipped authoring behavior, or can
  the replacement use platform/media playback for preview?
- Did any private platform backend consume FMOD-specific bank/event data not
  represented in `AudioBank.proto`?
- Did `Vitei.FMODInterop.dll` generate loop, channel, or stream metadata that
  differs from the repository's `WaveFileReader`?
- Are `crossfade`, rooms, or reverb settings remnants of an FMOD workflow, and
  should they be kept as source fields even if the open runtime ignores them?
- Are there FMOD license constraints that prevent retaining the binaries in the
  open toolchain?

## Binary-Only Gaps To Close

- Source for `AudioTool.exe`, `FSIDBuilder.exe`, and `AudioInterop.dll` is not
  present, so behavior must be reimplemented from metadata, focused IL
  inspection, and black-box fixtures.
- No engine-side sample `Data/Audio` or `Data/VPB/Audio` bank is checked in, so
  replacement tests need synthetic fixtures plus at least one recovered project
  fixture for parity.
- Legacy support for filters, effects, rooms, `filterCRC`, `effectCRCs`, and
  `roomNameCRC` is not explained by the old managed tool model. The replacement
  should preserve and edit the current runtime schema even though the legacy
  editor appears to have been sound-file focused.
- The editor's ATF/WinForms docking behavior is not worth reproducing; the data
  contract and builder output are the compatibility boundary.

## Migration Criteria

The replacement can become the default when:

- Existing audio YAML round-trips without semantic diffs.
- Generated FSID proto/header output matches legacy output for known fixtures.
- Generated CRCs match legacy IDs for all fixture banks.
- `rake platform=win build=debug data` succeeds for a project with audio banks.
- A runtime smoke test can load a bank, play a 2D sound, play a positional
  sound, play/stop music, and exercise a looped WAV.
- The README and build workflow no longer require binary-only audio authoring
  tools except for the temporary legacy fallback.
