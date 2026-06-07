using Usagi.ToolCore.Audio;
using Xunit;

namespace Usagi.ToolCore.Tests.Audio;

public sealed partial class FsidBuilderTests
{
    private const string FullAudioBankYaml = """
        AudioBank:
          soundFiles:
            - enumName: ROOM_TONE
              filename: room_tone
              stream: true
              loop: true
              volume: 0.75
              minDistance: 2.0
              maxDistance: 25.0
              eType: <%= AudioType::AUDIO_TYPE_SFX %>
              eFalloff: AUDIO_FALLOFF_LOGARITHMIC
              pitchRandomisation: 0.1
              dopplerFactor: 0.2
              basePitch: 1.1
              priority: 42
              crossfade: fade_a
              localized: false
              crc: 100
              filterCRC: 200
              effectCRCs:
                - 300
              roomNameCRC: 400
          filters:
            - enumName: FILTER_LOW
              crc: 200
              eFilter: AUDIO_FILTER_LOW_PASS
              fFrequency: 0.5
              fOneOverQ: 1.25
          reverbs:
            - effectDef:
                eEffectType: AUDIO_EFFECT_REVERB
                enumName: REVERB_SMALL
                crc: 300
              wetDryMix: 75.0
              reflectionsDelay: 10
              reverbDelay: 50
              roomFilterFreq: 100.0
              roomFilterMain: -3.0
              roomFilterHF: -6.0
              reflectionsGain: 5.0
              reverbGain: 6.0
              decayTime: 1.5
              density: 90.0
              roomSize: 25.0
          rooms:
            - roomName: ROOM_SMALL
              roomCrc: 400
              filterCrc: 200
              effectCrcs:
                - 300
        """;

    [Fact]
    public void ParseLegacyYaml_ReadsSoundFilesAndDefaults()
    {
        var bank = AudioBankYamlParser.ParseFile(FixturePath("minimal-legacy.yml"));

        Assert.Single(bank.SoundFiles);
        var sound = bank.SoundFiles[0];
        Assert.Equal("LASER_SHOT", sound.EnumName);
        Assert.Equal("laser_shot", sound.Filename);
        Assert.Equal(1.0f, sound.Volume);
        Assert.Equal(1000.0f, sound.MaxDistance);
        Assert.Equal(1, sound.AudioType);
        Assert.Equal(128, sound.Priority);
    }

    [Theory]
    [InlineData("LASER_SHOT", "LASER_SHOT", 615283986u)]
    [InlineData("Z_LAST", "Z_LAST", 4221454624u)]
    [InlineData(" spaced name ", "spaced_name", 1623057719u)]
    [InlineData("lowercase", "lowercase", 4265431538u)]
    public void ComputeSoundCrc_MatchesLegacyBuilder(string enumName, string normalizedName, uint expectedCrc)
    {
        Assert.Equal(normalizedName, FsidBuilder.NormalizeSoundName(enumName));
        Assert.Equal(expectedCrc, FsidBuilder.ComputeSoundCrc(enumName));
    }

    [Fact]
    public void WriteHeader_MatchesLegacyGoldenOutput()
    {
        var bank = AudioBankYamlParser.ParseFile(FixturePath("minimal-legacy.yml"));

        var header = FsidBuilder.WriteHeader(bank, "_CLR_TEST_AUDIO_FSID_", "TestAudio");

        Assert.Equal(Normalize(File.ReadAllText(FixturePath("expected.h"))), Normalize(header));
    }

    [Fact]
    public void WriteProto_MatchesLegacyGoldenOutput()
    {
        var bank = AudioBankYamlParser.ParseFile(FixturePath("minimal-legacy.yml"));

        var proto = FsidBuilder.WriteProto(bank, "TestAudio", new DateTimeOffset(2026, 5, 13, 20, 0, 0, TimeSpan.Zero));

        Assert.Equal(Normalize(File.ReadAllText(FixturePath("expected.proto"))), NormalizeProto(proto));
    }

    [Fact]
    public void WriteProto_PreservesOrderAndSignedCrcValues()
    {
        var bank = AudioBankYamlParser.ParseFile(FixturePath("edge-legacy.yml"));

        var proto = FsidBuilder.WriteProto(bank, "EdgeAudio", new DateTimeOffset(2026, 5, 13, 20, 0, 0, TimeSpan.Zero));

        Assert.Equal(Normalize(File.ReadAllText(FixturePath("expected-edge.proto"))), NormalizeProto(proto));
    }

    [Fact]
    public void WriteHeader_PreservesOrderAndUnsignedCrcValues()
    {
        var bank = AudioBankYamlParser.ParseFile(FixturePath("edge-legacy.yml"));

        var header = FsidBuilder.WriteHeader(bank, "_CLR_EDGE_AUDIO_FSID_", "EdgeAudio");

        Assert.Equal(Normalize(File.ReadAllText(FixturePath("expected-edge.h"))), Normalize(header));
    }

    [Fact]
    public void WriteHeader_RejectsDuplicateNormalizedNames()
    {
        var bank = AudioBankYamlParser.Parse("""
            AudioBank:
              soundFiles:
                - enumName: one sound
                  filename: one
                - enumName: one_sound
                  filename: two
            """);

        var error = Assert.Throws<InvalidOperationException>(() =>
            FsidBuilder.WriteHeader(bank, "_TEST_", "TestAudio"));
        Assert.Contains("Duplicate audio enum name", error.Message, StringComparison.Ordinal);
    }

    [Fact]
    public void ParseFullSchema_ReadsFiltersReverbsRoomsAndReferences()
    {
        var bank = AudioBankYamlParser.Parse(FullAudioBankYaml);

        Assert.Single(bank.SoundFiles);
        Assert.Single(bank.Filters);
        Assert.Single(bank.Reverbs);
        Assert.Single(bank.Rooms);

        var sound = bank.SoundFiles[0];
        Assert.Equal("ROOM_TONE", sound.EnumName);
        Assert.True(sound.Stream);
        Assert.True(sound.Loop);
        Assert.Equal(1, sound.AudioType);
        Assert.Equal(1, sound.Falloff);
        Assert.Equal(200u, sound.FilterCrc);
        Assert.Equal([300u], sound.EffectCrcs);
        Assert.Equal(400u, sound.RoomNameCrc);

        Assert.Equal("FILTER_LOW", bank.Filters[0].EnumName);
        Assert.Equal(0, bank.Filters[0].FilterType);
        Assert.Equal(0.5f, bank.Filters[0].Frequency);

        Assert.Equal("REVERB_SMALL", bank.Reverbs[0].Effect.EnumName);
        Assert.Equal(300u, bank.Reverbs[0].Effect.Crc);
        Assert.Equal(75.0f, bank.Reverbs[0].WetDryMix);

        Assert.Equal("ROOM_SMALL", bank.Rooms[0].RoomName);
        Assert.Equal([300u], bank.Rooms[0].EffectCrcs);
    }

    [Fact]
    public void WriteAudioBankYaml_RoundTripsFullSchema()
    {
        var original = AudioBankYamlParser.Parse(FullAudioBankYaml);

        var yaml = AudioBankYamlWriter.Write(original);
        var roundTripped = AudioBankYamlParser.Parse(yaml);

        Assert.Equal(original.SoundFiles[0].FilterCrc, roundTripped.SoundFiles[0].FilterCrc);
        Assert.Equal(original.SoundFiles[0].EffectCrcs, roundTripped.SoundFiles[0].EffectCrcs);
        Assert.Equal(original.Filters[0].OneOverQ, roundTripped.Filters[0].OneOverQ);
        Assert.Equal(original.Reverbs[0].RoomFilterHf, roundTripped.Reverbs[0].RoomFilterHf);
        Assert.Equal(original.Rooms[0].RoomCrc, roundTripped.Rooms[0].RoomCrc);
    }

    [Fact]
    public void WriteAudioBankYaml_EmitsExplicitEmptySequences()
    {
        var bank = AudioBankYamlParser.Parse("""
            AudioBank:
              soundFiles:
                - enumName: EMPTY_REFS
                  filename: empty_refs
                  effectCRCs: []
              filters: []
              reverbs: []
              rooms: []
            """);

        var yaml = AudioBankYamlWriter.Write(bank);

        Assert.Contains("effectCRCs: []", yaml, StringComparison.Ordinal);
        Assert.Contains("  filters: []", yaml, StringComparison.Ordinal);
        Assert.Contains("  reverbs: []", yaml, StringComparison.Ordinal);
        Assert.Contains("  rooms: []", yaml, StringComparison.Ordinal);
    }

    [Fact]
    public void ValidateAudioBank_AcceptsConsistentFullSchema()
    {
        var bank = AudioBankYamlParser.Parse(FullAudioBankYaml);

        var diagnostics = AudioBankValidator.Validate(bank);

        Assert.DoesNotContain(diagnostics, diagnostic => diagnostic.Severity == AudioBankDiagnosticSeverity.Error);
    }

    [Fact]
    public void ValidateAudioBank_ReportsBuildCriticalErrors()
    {
        var bank = AudioBankYamlParser.Parse("""
            AudioBank:
              soundFiles:
                - enumName: 1 invalid
                  filename: bad.wav
                  volume: -1
                  minDistance: 10
                  maxDistance: 1
                  priority: 999
                  effectCRCs: [1, 2, 3, 4, 5]
            """);

        var diagnostics = AudioBankValidator.Validate(bank);

        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].enumName");
        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].filename");
        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].volume");
        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].maxDistance");
        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].priority");
        Assert.Contains(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].effectCRCs");
    }

    private static string FixturePath(string fileName)
    {
        var root = FindRepoRoot();
        return Path.Combine(root, "Tools", "Tests", "AudioToolReverseEngineering", "FSIDBuilderSmoke", fileName);
    }

    private static string FindRepoRoot()
    {
        var directory = new DirectoryInfo(AppContext.BaseDirectory);
        while (directory is not null)
        {
            if (Directory.Exists(Path.Combine(directory.FullName, "Tools", "Tests", "AudioToolReverseEngineering")))
            {
                return directory.FullName;
            }

            directory = directory.Parent;
        }

        throw new DirectoryNotFoundException("Could not locate Usagi repository root.");
    }

    private static string NormalizeProto(string value)
    {
        value = value.Replace("\r\n", "\n", StringComparison.Ordinal);
        value = value.Replace("Copyright \u00a9 Vitei", "Copyright (c) Vitei", StringComparison.Ordinal);
        return TimestampLineRegex().Replace(value, "//  Auto-generated by Usagi Audio Tool at <timestamp>");
    }

    private static string Normalize(string value) =>
        value.Replace("\r\n", "\n", StringComparison.Ordinal);

    [System.Text.RegularExpressions.GeneratedRegex("//  Auto-generated by Usagi Audio Tool at .+")]
    private static partial System.Text.RegularExpressions.Regex TimestampLineRegex();
}
