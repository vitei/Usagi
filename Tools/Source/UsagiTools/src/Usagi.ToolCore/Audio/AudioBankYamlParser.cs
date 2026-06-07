using YamlDotNet.RepresentationModel;

namespace Usagi.ToolCore.Audio;

public static class AudioBankYamlParser
{
    private static readonly IReadOnlyDictionary<string, int> AudioTypes = new Dictionary<string, int>(StringComparer.Ordinal)
    {
        ["AUDIO_TYPE_MUSIC"] = 0,
        ["AUDIO_TYPE_SFX"] = 1,
        ["AUDIO_TYPE_UI"] = 2,
        ["AUDIO_TYPE_SPEECH"] = 3,
        ["AUDIO_TYPE_CUSTOM"] = 4
    };

    private static readonly IReadOnlyDictionary<string, int> AudioFalloffs = new Dictionary<string, int>(StringComparer.Ordinal)
    {
        ["AUDIO_FALLOFF_LINEAR"] = 0,
        ["AUDIO_FALLOFF_LOGARITHMIC"] = 1
    };

    private static readonly IReadOnlyDictionary<string, int> AudioFilters = new Dictionary<string, int>(StringComparer.Ordinal)
    {
        ["AUDIO_FILTER_LOW_PASS"] = 0,
        ["AUDIO_FILTER_HIGH_PASS"] = 1
    };

    private static readonly IReadOnlyDictionary<string, int> AudioEffects = new Dictionary<string, int>(StringComparer.Ordinal)
    {
        ["AUDIO_EFFECT_REVERB"] = 0
    };

    public static AudioBank ParseFile(string path)
    {
        var content = File.ReadAllText(path);
        return Parse(content);
    }

    public static AudioBank Parse(string yaml)
    {
        var stream = new YamlStream();
        stream.Load(new StringReader(yaml));

        var bank = new AudioBank();
        if (stream.Documents.Count == 0 ||
            stream.Documents[0].RootNode is not YamlMappingNode root ||
            !TryGetMapping(root, "AudioBank", out var audioBankNode))
        {
            return bank;
        }

        if (!TryGetSequence(audioBankNode, "soundFiles", out var soundFiles))
        {
            return bank;
        }

        foreach (var item in soundFiles.Children)
        {
            if (item is YamlMappingNode soundFileNode)
            {
                bank.SoundFiles.Add(ParseSoundFile(soundFileNode));
            }
        }

        if (TryGetSequence(audioBankNode, "filters", out var filters))
        {
            foreach (var item in filters.Children)
            {
                if (item is YamlMappingNode filterNode)
                {
                    bank.Filters.Add(ParseFilter(filterNode));
                }
            }
        }

        if (TryGetSequence(audioBankNode, "reverbs", out var reverbs))
        {
            foreach (var item in reverbs.Children)
            {
                if (item is YamlMappingNode reverbNode)
                {
                    bank.Reverbs.Add(ParseReverb(reverbNode));
                }
            }
        }

        if (TryGetSequence(audioBankNode, "rooms", out var rooms))
        {
            foreach (var item in rooms.Children)
            {
                if (item is YamlMappingNode roomNode)
                {
                    bank.Rooms.Add(ParseRoom(roomNode));
                }
            }
        }

        return bank;
    }

    private static SoundFileDefinition ParseSoundFile(YamlMappingNode node)
    {
        var sound = new SoundFileDefinition();
        foreach (var entry in node.Children)
        {
            var key = Scalar(entry.Key);
            var value = Scalar(entry.Value);
            switch (key)
            {
                case "enumName":
                    sound.EnumName = value;
                    break;
                case "filename":
                    sound.Filename = value;
                    break;
                case "stream":
                    sound.Stream = ParseBool(value);
                    break;
                case "loop":
                    sound.Loop = ParseBool(value);
                    break;
                case "volume":
                    sound.Volume = SoundFileDefinition.ParseFloat(value, sound.Volume);
                    break;
                case "minDistance":
                    sound.MinDistance = SoundFileDefinition.ParseFloat(value, sound.MinDistance);
                    break;
                case "maxDistance":
                    sound.MaxDistance = SoundFileDefinition.ParseFloat(value, sound.MaxDistance);
                    break;
                case "eType":
                    sound.AudioType = ParseEnumInt(value, sound.AudioType, AudioTypes);
                    break;
                case "eFalloff":
                    sound.Falloff = ParseEnumInt(value, sound.Falloff, AudioFalloffs);
                    break;
                case "pitchRandomisation":
                    sound.PitchRandomisation = SoundFileDefinition.ParseFloat(value, sound.PitchRandomisation);
                    break;
                case "priority":
                    sound.Priority = ParseInt(value, sound.Priority);
                    break;
                case "crossfade":
                    sound.Crossfade = value;
                    break;
                case "basePitch":
                    sound.BasePitch = SoundFileDefinition.ParseFloat(value, sound.BasePitch);
                    break;
                case "dopplerFactor":
                    sound.DopplerFactor = SoundFileDefinition.ParseFloat(value, sound.DopplerFactor);
                    break;
                case "localized":
                    sound.Localized = ParseBool(value);
                    break;
                case "crc":
                    sound.Crc = ParseUInt(value, sound.Crc);
                    break;
                case "filterCRC":
                    sound.FilterCrc = ParseUInt(value, sound.FilterCrc);
                    break;
                case "effectCRCs" when entry.Value is YamlSequenceNode effectCrcs:
                    sound.EffectCrcs.Clear();
                    foreach (var effectCrc in effectCrcs.Children)
                    {
                        sound.EffectCrcs.Add(ParseUInt(Scalar(effectCrc), 0));
                    }
                    break;
                case "roomNameCRC":
                    sound.RoomNameCrc = ParseUInt(value, sound.RoomNameCrc);
                    break;
            }
        }

        return sound;
    }

    private static AudioFilterDefinition ParseFilter(YamlMappingNode node)
    {
        var filter = new AudioFilterDefinition();
        foreach (var entry in node.Children)
        {
            var key = Scalar(entry.Key);
            var value = Scalar(entry.Value);
            switch (key)
            {
                case "enumName":
                    filter.EnumName = value;
                    break;
                case "crc":
                    filter.Crc = ParseUInt(value, filter.Crc);
                    break;
                case "eFilter":
                    filter.FilterType = ParseEnumInt(value, filter.FilterType, AudioFilters);
                    break;
                case "fFrequency":
                    filter.Frequency = SoundFileDefinition.ParseFloat(value, filter.Frequency);
                    break;
                case "fOneOverQ":
                    filter.OneOverQ = SoundFileDefinition.ParseFloat(value, filter.OneOverQ);
                    break;
            }
        }

        return filter;
    }

    private static ReverbEffectDefinition ParseReverb(YamlMappingNode node)
    {
        var reverb = new ReverbEffectDefinition();
        foreach (var entry in node.Children)
        {
            var key = Scalar(entry.Key);
            var value = Scalar(entry.Value);
            switch (key)
            {
                case "effectDef" when entry.Value is YamlMappingNode effect:
                    ParseEffect(effect, reverb.Effect);
                    break;
                case "wetDryMix":
                    reverb.WetDryMix = SoundFileDefinition.ParseFloat(value, reverb.WetDryMix);
                    break;
                case "reflectionsDelay":
                    reverb.ReflectionsDelay = ParseInt(value, reverb.ReflectionsDelay);
                    break;
                case "reverbDelay":
                    reverb.ReverbDelay = ParseInt(value, reverb.ReverbDelay);
                    break;
                case "roomFilterFreq":
                    reverb.RoomFilterFreq = SoundFileDefinition.ParseFloat(value, reverb.RoomFilterFreq);
                    break;
                case "roomFilterMain":
                    reverb.RoomFilterMain = SoundFileDefinition.ParseFloat(value, reverb.RoomFilterMain);
                    break;
                case "roomFilterHF":
                    reverb.RoomFilterHf = SoundFileDefinition.ParseFloat(value, reverb.RoomFilterHf);
                    break;
                case "reflectionsGain":
                    reverb.ReflectionsGain = SoundFileDefinition.ParseFloat(value, reverb.ReflectionsGain);
                    break;
                case "reverbGain":
                    reverb.ReverbGain = SoundFileDefinition.ParseFloat(value, reverb.ReverbGain);
                    break;
                case "decayTime":
                    reverb.DecayTime = SoundFileDefinition.ParseFloat(value, reverb.DecayTime);
                    break;
                case "density":
                    reverb.Density = SoundFileDefinition.ParseFloat(value, reverb.Density);
                    break;
                case "roomSize":
                    reverb.RoomSize = SoundFileDefinition.ParseFloat(value, reverb.RoomSize);
                    break;
            }
        }

        return reverb;
    }

    private static void ParseEffect(YamlMappingNode node, AudioEffectDefinition effect)
    {
        foreach (var entry in node.Children)
        {
            var key = Scalar(entry.Key);
            var value = Scalar(entry.Value);
            switch (key)
            {
                case "eEffectType":
                    effect.EffectType = ParseEnumInt(value, effect.EffectType, AudioEffects);
                    break;
                case "enumName":
                    effect.EnumName = value;
                    break;
                case "crc":
                    effect.Crc = ParseUInt(value, effect.Crc);
                    break;
            }
        }
    }

    private static AudioRoomDefinition ParseRoom(YamlMappingNode node)
    {
        var room = new AudioRoomDefinition();
        foreach (var entry in node.Children)
        {
            var key = Scalar(entry.Key);
            var value = Scalar(entry.Value);
            switch (key)
            {
                case "roomName":
                    room.RoomName = value;
                    break;
                case "roomCrc":
                    room.RoomCrc = ParseUInt(value, room.RoomCrc);
                    break;
                case "filterCrc":
                    room.FilterCrc = ParseUInt(value, room.FilterCrc);
                    break;
                case "effectCrcs" when entry.Value is YamlSequenceNode effectCrcs:
                    room.EffectCrcs.Clear();
                    foreach (var effectCrc in effectCrcs.Children)
                    {
                        room.EffectCrcs.Add(ParseUInt(Scalar(effectCrc), 0));
                    }
                    break;
            }
        }

        return room;
    }

    private static bool TryGetMapping(YamlMappingNode node, string key, out YamlMappingNode mapping)
    {
        if (node.Children.TryGetValue(new YamlScalarNode(key), out var value) && value is YamlMappingNode found)
        {
            mapping = found;
            return true;
        }

        mapping = new YamlMappingNode();
        return false;
    }

    private static bool TryGetSequence(YamlMappingNode node, string key, out YamlSequenceNode sequence)
    {
        if (node.Children.TryGetValue(new YamlScalarNode(key), out var value) && value is YamlSequenceNode found)
        {
            sequence = found;
            return true;
        }

        sequence = new YamlSequenceNode();
        return false;
    }

    private static string Scalar(YamlNode node) =>
        node is YamlScalarNode scalar ? scalar.Value ?? "" : "";

    private static bool ParseBool(string value) =>
        value.Equals("true", StringComparison.OrdinalIgnoreCase);

    private static int ParseInt(string value, int fallback) =>
        int.TryParse(value, out var parsed) ? parsed : fallback;

    private static uint ParseUInt(string value, uint fallback) =>
        uint.TryParse(value, out var parsed) ? parsed : fallback;

    private static int ParseEnumInt(string value, int fallback, IReadOnlyDictionary<string, int> values)
    {
        if (int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        var name = value.Trim();
        if (name.StartsWith("<%=", StringComparison.Ordinal) && name.EndsWith("%>", StringComparison.Ordinal))
        {
            name = name[3..^2].Trim();
        }

        var separator = name.LastIndexOf("::", StringComparison.Ordinal);
        if (separator >= 0)
        {
            name = name[(separator + 2)..];
        }

        return values.TryGetValue(name, out var mapped) ? mapped : fallback;
    }
}
