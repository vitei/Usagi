using System.Globalization;

namespace Usagi.ToolCore.Audio;

public sealed class AudioBank
{
    public List<SoundFileDefinition> SoundFiles { get; } = [];
    public List<AudioFilterDefinition> Filters { get; } = [];
    public List<ReverbEffectDefinition> Reverbs { get; } = [];
    public List<AudioRoomDefinition> Rooms { get; } = [];
}

public sealed class SoundFileDefinition
{
    public string EnumName { get; set; } = "";
    public string Filename { get; set; } = "";
    public bool Stream { get; set; }
    public bool Loop { get; set; }
    public float Volume { get; set; } = 1.0f;
    public float MinDistance { get; set; } = 1.0f;
    public float MaxDistance { get; set; } = 1000.0f;
    public int AudioType { get; set; } = 1;
    public int Falloff { get; set; }
    public float PitchRandomisation { get; set; }
    public int Priority { get; set; } = 128;
    public string Crossfade { get; set; } = "";
    public float BasePitch { get; set; } = 1.0f;
    public float DopplerFactor { get; set; }
    public bool Localized { get; set; }
    public uint Crc { get; set; }
    public uint FilterCrc { get; set; }
    public List<uint> EffectCrcs { get; } = [];
    public uint RoomNameCrc { get; set; }

    internal static float ParseFloat(string value, float fallback)
    {
        return float.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var parsed)
            ? parsed
            : fallback;
    }
}

public sealed class AudioFilterDefinition
{
    public string EnumName { get; set; } = "";
    public uint Crc { get; set; }
    public int FilterType { get; set; }
    public float Frequency { get; set; }
    public float OneOverQ { get; set; }
}

public sealed class AudioEffectDefinition
{
    public int EffectType { get; set; }
    public string EnumName { get; set; } = "";
    public uint Crc { get; set; }
}

public sealed class ReverbEffectDefinition
{
    public AudioEffectDefinition Effect { get; } = new();
    public float WetDryMix { get; set; } = 100.0f;
    public int ReflectionsDelay { get; set; }
    public int ReverbDelay { get; set; } = 85;
    public float RoomFilterFreq { get; set; } = 20.0f;
    public float RoomFilterMain { get; set; }
    public float RoomFilterHf { get; set; }
    public float ReflectionsGain { get; set; } = 20.0f;
    public float ReverbGain { get; set; } = 20.0f;
    public float DecayTime { get; set; } = 1.0f;
    public float Density { get; set; } = 100.0f;
    public float RoomSize { get; set; } = 100.0f;
}

public sealed class AudioRoomDefinition
{
    public string RoomName { get; set; } = "";
    public uint RoomCrc { get; set; }
    public uint FilterCrc { get; set; }
    public List<uint> EffectCrcs { get; } = [];
}
