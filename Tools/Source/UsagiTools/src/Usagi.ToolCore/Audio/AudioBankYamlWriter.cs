using System.Globalization;
using System.Text;

namespace Usagi.ToolCore.Audio;

public static class AudioBankYamlWriter
{
    public static string Write(AudioBank bank)
    {
        var writer = new StringBuilder();
        writer.AppendLine("AudioBank:");
        WriteSoundFiles(writer, bank.SoundFiles);
        WriteFilters(writer, bank.Filters);
        WriteReverbs(writer, bank.Reverbs);
        WriteRooms(writer, bank.Rooms);
        return writer.ToString();
    }

    private static void WriteSoundFiles(StringBuilder writer, IReadOnlyCollection<SoundFileDefinition> soundFiles)
    {
        if (soundFiles.Count == 0)
        {
            writer.AppendLine("  soundFiles: []");
            return;
        }

        writer.AppendLine("  soundFiles:");
        foreach (var sound in soundFiles)
        {
            writer.AppendLine($"    - enumName: {Quote(sound.EnumName)}");
            writer.AppendLine($"      filename: {Quote(sound.Filename)}");
            writer.AppendLine($"      stream: {Bool(sound.Stream)}");
            writer.AppendLine($"      loop: {Bool(sound.Loop)}");
            writer.AppendLine($"      volume: {Float(sound.Volume)}");
            writer.AppendLine($"      minDistance: {Float(sound.MinDistance)}");
            writer.AppendLine($"      maxDistance: {Float(sound.MaxDistance)}");
            writer.AppendLine($"      eType: {sound.AudioType}");
            writer.AppendLine($"      eFalloff: {sound.Falloff}");
            writer.AppendLine($"      pitchRandomisation: {Float(sound.PitchRandomisation)}");
            writer.AppendLine($"      dopplerFactor: {Float(sound.DopplerFactor)}");
            writer.AppendLine($"      basePitch: {Float(sound.BasePitch)}");
            writer.AppendLine($"      priority: {sound.Priority}");
            writer.AppendLine($"      crossfade: {Quote(sound.Crossfade)}");
            writer.AppendLine($"      localized: {Bool(sound.Localized)}");
            writer.AppendLine($"      crc: {sound.Crc}");
            writer.AppendLine($"      filterCRC: {sound.FilterCrc}");
            WriteUIntList(writer, "effectCRCs", sound.EffectCrcs, 6);
            writer.AppendLine($"      roomNameCRC: {sound.RoomNameCrc}");
        }
    }

    private static void WriteFilters(StringBuilder writer, IReadOnlyCollection<AudioFilterDefinition> filters)
    {
        if (filters.Count == 0)
        {
            writer.AppendLine("  filters: []");
            return;
        }

        writer.AppendLine("  filters:");
        foreach (var filter in filters)
        {
            writer.AppendLine($"    - enumName: {Quote(filter.EnumName)}");
            writer.AppendLine($"      crc: {filter.Crc}");
            writer.AppendLine($"      eFilter: {filter.FilterType}");
            writer.AppendLine($"      fFrequency: {Float(filter.Frequency)}");
            writer.AppendLine($"      fOneOverQ: {Float(filter.OneOverQ)}");
        }
    }

    private static void WriteReverbs(StringBuilder writer, IReadOnlyCollection<ReverbEffectDefinition> reverbs)
    {
        if (reverbs.Count == 0)
        {
            writer.AppendLine("  reverbs: []");
            return;
        }

        writer.AppendLine("  reverbs:");
        foreach (var reverb in reverbs)
        {
            writer.AppendLine("    - effectDef:");
            writer.AppendLine($"        eEffectType: {reverb.Effect.EffectType}");
            writer.AppendLine($"        enumName: {Quote(reverb.Effect.EnumName)}");
            writer.AppendLine($"        crc: {reverb.Effect.Crc}");
            writer.AppendLine($"      wetDryMix: {Float(reverb.WetDryMix)}");
            writer.AppendLine($"      reflectionsDelay: {reverb.ReflectionsDelay}");
            writer.AppendLine($"      reverbDelay: {reverb.ReverbDelay}");
            writer.AppendLine($"      roomFilterFreq: {Float(reverb.RoomFilterFreq)}");
            writer.AppendLine($"      roomFilterMain: {Float(reverb.RoomFilterMain)}");
            writer.AppendLine($"      roomFilterHF: {Float(reverb.RoomFilterHf)}");
            writer.AppendLine($"      reflectionsGain: {Float(reverb.ReflectionsGain)}");
            writer.AppendLine($"      reverbGain: {Float(reverb.ReverbGain)}");
            writer.AppendLine($"      decayTime: {Float(reverb.DecayTime)}");
            writer.AppendLine($"      density: {Float(reverb.Density)}");
            writer.AppendLine($"      roomSize: {Float(reverb.RoomSize)}");
        }
    }

    private static void WriteRooms(StringBuilder writer, IReadOnlyCollection<AudioRoomDefinition> rooms)
    {
        if (rooms.Count == 0)
        {
            writer.AppendLine("  rooms: []");
            return;
        }

        writer.AppendLine("  rooms:");
        foreach (var room in rooms)
        {
            writer.AppendLine($"    - roomName: {Quote(room.RoomName)}");
            writer.AppendLine($"      roomCrc: {room.RoomCrc}");
            writer.AppendLine($"      filterCrc: {room.FilterCrc}");
            WriteUIntList(writer, "effectCrcs", room.EffectCrcs, 6);
        }
    }

    private static void WriteUIntList(StringBuilder writer, string name, IReadOnlyCollection<uint> values, int indent)
    {
        var spaces = new string(' ', indent);
        if (values.Count == 0)
        {
            writer.Append(spaces).Append(name).AppendLine(": []");
            return;
        }

        writer.Append(spaces).Append(name).AppendLine(":");
        foreach (var value in values)
        {
            writer.Append(spaces).Append("  - ").Append(value).AppendLine();
        }
    }

    private static string Quote(string value) =>
        "\"" + value.Replace("\\", "\\\\", StringComparison.Ordinal).Replace("\"", "\\\"", StringComparison.Ordinal) + "\"";

    private static string Bool(bool value) => value ? "true" : "false";

    private static string Float(float value) => value.ToString("0.########", CultureInfo.InvariantCulture);
}
