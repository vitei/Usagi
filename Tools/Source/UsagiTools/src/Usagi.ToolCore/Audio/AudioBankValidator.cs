using System.Text.RegularExpressions;

namespace Usagi.ToolCore.Audio;

public static partial class AudioBankValidator
{
    public static IReadOnlyList<AudioBankDiagnostic> Validate(AudioBank bank, string? projectRoot = null)
    {
        var diagnostics = new List<AudioBankDiagnostic>();
        var soundNames = new HashSet<string>(StringComparer.Ordinal);
        var filterCrcs = bank.Filters.Select(filter => filter.Crc).Where(crc => crc != 0).ToHashSet();
        var effectCrcs = bank.Reverbs.Select(reverb => reverb.Effect.Crc).Where(crc => crc != 0).ToHashSet();
        var roomCrcs = bank.Rooms.Select(room => room.RoomCrc).Where(crc => crc != 0).ToHashSet();

        for (var i = 0; i < bank.SoundFiles.Count; i++)
        {
            ValidateSoundFile(bank.SoundFiles[i], i, diagnostics, soundNames, filterCrcs, effectCrcs, roomCrcs, projectRoot);
        }

        for (var i = 0; i < bank.Filters.Count; i++)
        {
            ValidateNamedCrc("filters", i, bank.Filters[i].EnumName, bank.Filters[i].Crc, diagnostics);
        }

        for (var i = 0; i < bank.Reverbs.Count; i++)
        {
            ValidateNamedCrc("reverbs", i, bank.Reverbs[i].Effect.EnumName, bank.Reverbs[i].Effect.Crc, diagnostics);
        }

        for (var i = 0; i < bank.Rooms.Count; i++)
        {
            ValidateNamedCrc("rooms", i, bank.Rooms[i].RoomName, bank.Rooms[i].RoomCrc, diagnostics);
            if (bank.Rooms[i].EffectCrcs.Count > 4)
            {
                diagnostics.Add(Error($"rooms[{i}].effectCrcs", "Audio rooms may reference at most four effects."));
            }
        }

        return diagnostics;
    }

    private static void ValidateSoundFile(
        SoundFileDefinition sound,
        int index,
        List<AudioBankDiagnostic> diagnostics,
        HashSet<string> soundNames,
        HashSet<uint> filterCrcs,
        HashSet<uint> effectCrcs,
        HashSet<uint> roomCrcs,
        string? projectRoot)
    {
        var field = $"soundFiles[{index}]";
        var normalizedName = FsidBuilder.NormalizeSoundName(sound.EnumName);
        if (string.IsNullOrWhiteSpace(sound.EnumName))
        {
            diagnostics.Add(Error($"{field}.enumName", "Sound enum name is required."));
        }
        else if (!IdentifierRegex().IsMatch(normalizedName))
        {
            diagnostics.Add(Error($"{field}.enumName", $"Sound enum name '{sound.EnumName}' normalizes to invalid identifier '{normalizedName}'."));
        }
        else if (!soundNames.Add(normalizedName))
        {
            diagnostics.Add(Error($"{field}.enumName", $"Duplicate normalized sound enum name '{normalizedName}'."));
        }

        if (sound.EnumName.Length > 32)
        {
            diagnostics.Add(Error($"{field}.enumName", "Sound enum name exceeds nanopb max size 32."));
        }

        if (string.IsNullOrWhiteSpace(sound.Filename))
        {
            diagnostics.Add(Error($"{field}.filename", "Sound filename is required."));
        }
        else
        {
            if (Path.HasExtension(sound.Filename))
            {
                diagnostics.Add(Error($"{field}.filename", "Sound filename must be extensionless."));
            }

            if (sound.Filename.Length > 32)
            {
                diagnostics.Add(Error($"{field}.filename", "Sound filename exceeds nanopb max size 32."));
            }

            if (projectRoot is not null)
            {
                var wavPath = Path.Combine(projectRoot, "Data", "Audio", sound.Filename + ".wav");
                if (!File.Exists(wavPath))
                {
                    diagnostics.Add(Warning($"{field}.filename", $"Referenced WAV was not found at {wavPath}."));
                }
                else
                {
                    ValidateWaveMetadata(sound, field, wavPath, diagnostics);
                }
            }
        }

        if (sound.Volume < 0)
        {
            diagnostics.Add(Error($"{field}.volume", "Volume must be non-negative."));
        }

        if (sound.MaxDistance < sound.MinDistance)
        {
            diagnostics.Add(Error($"{field}.maxDistance", "Max distance must be greater than or equal to min distance."));
        }

        if (sound.Priority is < 0 or > 255)
        {
            diagnostics.Add(Error($"{field}.priority", "Priority must be between 0 and 255."));
        }

        if (sound.EffectCrcs.Count > 4)
        {
            diagnostics.Add(Error($"{field}.effectCRCs", "Sound files may reference at most four effects."));
        }

        if (sound.FilterCrc != 0 && !filterCrcs.Contains(sound.FilterCrc))
        {
            diagnostics.Add(Warning($"{field}.filterCRC", $"Filter CRC {sound.FilterCrc} does not match a filter in this bank."));
        }

        foreach (var effectCrc in sound.EffectCrcs.Where(effectCrc => effectCrc != 0 && !effectCrcs.Contains(effectCrc)))
        {
            diagnostics.Add(Warning($"{field}.effectCRCs", $"Effect CRC {effectCrc} does not match a reverb in this bank."));
        }

        if (sound.RoomNameCrc != 0 && !roomCrcs.Contains(sound.RoomNameCrc))
        {
            diagnostics.Add(Warning($"{field}.roomNameCRC", $"Room CRC {sound.RoomNameCrc} does not match a room in this bank."));
        }
    }

    private static void ValidateWaveMetadata(
        SoundFileDefinition sound,
        string field,
        string wavPath,
        List<AudioBankDiagnostic> diagnostics)
    {
        try
        {
            var metadata = WaveMetadataReader.ReadFile(wavPath);
            if (sound.Loop && !metadata.HasForwardLoop)
            {
                diagnostics.Add(Warning($"{field}.loop", $"Looping sound has no forward smpl loop in {wavPath}."));
            }

            if (metadata.FormatId != 1)
            {
                diagnostics.Add(Warning($"{field}.filename", $"WAV format {metadata.FormatId} is not PCM."));
            }

            if (metadata.ChannelCount == 0 || metadata.SampleRate == 0 || metadata.BitsPerSample == 0)
            {
                diagnostics.Add(Warning($"{field}.filename", $"WAV metadata is incomplete for {wavPath}."));
            }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            diagnostics.Add(Warning($"{field}.filename", $"Could not parse WAV metadata for {wavPath}: {ex.Message}"));
        }
    }

    private static void ValidateNamedCrc(string collection, int index, string name, uint crc, List<AudioBankDiagnostic> diagnostics)
    {
        var field = $"{collection}[{index}]";
        if (string.IsNullOrWhiteSpace(name))
        {
            diagnostics.Add(Error($"{field}.enumName", "Name is required."));
        }
        else if (name.Length > 32)
        {
            diagnostics.Add(Error($"{field}.enumName", "Name exceeds nanopb max size 32."));
        }

        if (crc == 0)
        {
            diagnostics.Add(Warning($"{field}.crc", "CRC is zero; references to this object will not resolve."));
        }
    }

    private static AudioBankDiagnostic Error(string field, string message) =>
        new(AudioBankDiagnosticSeverity.Error, field, message);

    private static AudioBankDiagnostic Warning(string field, string message) =>
        new(AudioBankDiagnosticSeverity.Warning, field, message);

    [GeneratedRegex(@"^[A-Za-z_][A-Za-z0-9_]*$", RegexOptions.CultureInvariant)]
    private static partial Regex IdentifierRegex();
}

public readonly record struct AudioBankDiagnostic(
    AudioBankDiagnosticSeverity Severity,
    string Field,
    string Message);

public enum AudioBankDiagnosticSeverity
{
    Warning,
    Error
}
