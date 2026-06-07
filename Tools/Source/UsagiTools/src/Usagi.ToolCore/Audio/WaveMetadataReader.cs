using System.Buffers.Binary;

namespace Usagi.ToolCore.Audio;

public static class WaveMetadataReader
{
    private const uint LoopTypeForward = 0;

    public static WaveMetadata ReadFile(string path)
    {
        var bytes = File.ReadAllBytes(path);
        return Read(bytes, path);
    }

    public static WaveMetadata Read(ReadOnlySpan<byte> data, string sourcePath = "")
    {
        if (data.Length < 12)
        {
            throw new InvalidDataException("WAV data is too small to contain a RIFF header.");
        }

        if (!HasTag(data, 0, "RIFF") || !HasTag(data, 8, "WAVE"))
        {
            throw new InvalidDataException("WAV data is not a RIFF/WAVE file.");
        }

        ushort? formatId = null;
        ushort? channelCount = null;
        uint? sampleRate = null;
        uint? byteRate = null;
        ushort? blockAlign = null;
        ushort? bitsPerSample = null;
        uint? dataSize = null;
        uint? dataOffset = null;
        WaveLoop? firstForwardLoop = null;

        var offset = 12;
        while (offset + 8 <= data.Length)
        {
            var tag = ReadTag(data, offset);
            var chunkSize = BinaryPrimitives.ReadUInt32LittleEndian(data[(offset + 4)..(offset + 8)]);
            var payloadOffset = offset + 8;
            if (chunkSize > int.MaxValue || payloadOffset + (int)chunkSize > data.Length)
            {
                throw new InvalidDataException($"WAV chunk '{tag}' extends beyond the end of the file.");
            }

            var payload = data.Slice(payloadOffset, (int)chunkSize);
            if (tag == "fmt ")
            {
                if (payload.Length < 16)
                {
                    throw new InvalidDataException("WAV fmt chunk is smaller than 16 bytes.");
                }

                formatId = BinaryPrimitives.ReadUInt16LittleEndian(payload[0..2]);
                channelCount = BinaryPrimitives.ReadUInt16LittleEndian(payload[2..4]);
                sampleRate = BinaryPrimitives.ReadUInt32LittleEndian(payload[4..8]);
                byteRate = BinaryPrimitives.ReadUInt32LittleEndian(payload[8..12]);
                blockAlign = BinaryPrimitives.ReadUInt16LittleEndian(payload[12..14]);
                bitsPerSample = BinaryPrimitives.ReadUInt16LittleEndian(payload[14..16]);
            }
            else if (tag == "data")
            {
                dataSize = chunkSize;
                dataOffset = (uint)payloadOffset;
            }
            else if (tag == "smpl" && payload.Length >= 36)
            {
                var loopCount = BinaryPrimitives.ReadUInt32LittleEndian(payload[28..32]);
                var loopOffset = 36;
                for (uint i = 0; i < loopCount && loopOffset + 24 <= payload.Length; i++, loopOffset += 24)
                {
                    var loopPayload = payload.Slice(loopOffset, 24);
                    var type = BinaryPrimitives.ReadUInt32LittleEndian(loopPayload[4..8]);
                    if (type == LoopTypeForward)
                    {
                        var start = BinaryPrimitives.ReadUInt32LittleEndian(loopPayload[8..12]);
                        var end = BinaryPrimitives.ReadUInt32LittleEndian(loopPayload[12..16]);
                        firstForwardLoop = new WaveLoop(start, end, end >= start ? end - start + 1 : 0);
                        break;
                    }
                }
            }

            offset = payloadOffset + (int)chunkSize;
            if ((chunkSize & 1) != 0)
            {
                offset++;
            }
        }

        if (formatId is null || channelCount is null || sampleRate is null || byteRate is null ||
            blockAlign is null || bitsPerSample is null)
        {
            throw new InvalidDataException("WAV fmt chunk was not found.");
        }

        if (dataSize is null || dataOffset is null)
        {
            throw new InvalidDataException("WAV data chunk was not found.");
        }

        var duration = byteRate.Value == 0
            ? TimeSpan.Zero
            : TimeSpan.FromSeconds(dataSize.Value / (double)byteRate.Value);

        return new WaveMetadata(
            sourcePath,
            formatId.Value,
            channelCount.Value,
            sampleRate.Value,
            byteRate.Value,
            blockAlign.Value,
            bitsPerSample.Value,
            dataSize.Value,
            dataOffset.Value,
            duration,
            firstForwardLoop);
    }

    private static bool HasTag(ReadOnlySpan<byte> data, int offset, string expected)
    {
        return offset + 4 <= data.Length && ReadTag(data, offset) == expected;
    }

    private static string ReadTag(ReadOnlySpan<byte> data, int offset)
    {
        return new string([
            (char)data[offset],
            (char)data[offset + 1],
            (char)data[offset + 2],
            (char)data[offset + 3]
        ]);
    }
}

public sealed record WaveMetadata(
    string SourcePath,
    ushort FormatId,
    ushort ChannelCount,
    uint SampleRate,
    uint ByteRate,
    ushort BlockAlign,
    ushort BitsPerSample,
    uint DataSize,
    uint DataOffset,
    TimeSpan Duration,
    WaveLoop? ForwardLoop)
{
    public bool HasForwardLoop => ForwardLoop is not null;
}

public sealed record WaveLoop(uint Start, uint End, uint Length);
