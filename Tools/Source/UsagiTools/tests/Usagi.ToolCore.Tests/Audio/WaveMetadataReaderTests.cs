using System.Buffers.Binary;
using Usagi.ToolCore.Audio;
using Xunit;

namespace Usagi.ToolCore.Tests.Audio;

public sealed class WaveMetadataReaderTests
{
    [Fact]
    public void Read_ParsesPcmFmtAndDataChunks()
    {
        var wav = CreateWave(loop: null);

        var metadata = WaveMetadataReader.Read(wav);

        Assert.Equal(1, metadata.FormatId);
        Assert.Equal(2, metadata.ChannelCount);
        Assert.Equal(48_000u, metadata.SampleRate);
        Assert.Equal(192_000u, metadata.ByteRate);
        Assert.Equal(4, metadata.BlockAlign);
        Assert.Equal(16, metadata.BitsPerSample);
        Assert.Equal(16u, metadata.DataSize);
        Assert.Equal(TimeSpan.FromSeconds(16.0 / 192_000.0), metadata.Duration);
        Assert.Null(metadata.ForwardLoop);
    }

    [Fact]
    public void Read_ParsesFirstForwardSmplLoopLikeRuntime()
    {
        var wav = CreateWave(new LoopSpec(Type: 0, Start: 10, End: 20));

        var metadata = WaveMetadataReader.Read(wav);

        Assert.NotNull(metadata.ForwardLoop);
        Assert.Equal(10u, metadata.ForwardLoop.Start);
        Assert.Equal(20u, metadata.ForwardLoop.End);
        Assert.Equal(11u, metadata.ForwardLoop.Length);
    }

    [Fact]
    public void Read_IgnoresNonForwardSmplLoops()
    {
        var wav = CreateWave(new LoopSpec(Type: 1, Start: 10, End: 20));

        var metadata = WaveMetadataReader.Read(wav);

        Assert.Null(metadata.ForwardLoop);
    }

    [Fact]
    public void ValidateAudioBank_WarnsWhenLoopingWavHasNoForwardLoop()
    {
        var root = CreateTempProjectRoot(CreateWave(loop: null));
        var bank = AudioBankYamlParser.Parse("""
            AudioBank:
              soundFiles:
                - enumName: LOOPING
                  filename: looping
                  loop: true
            """);

        var diagnostics = AudioBankValidator.Validate(bank, root);

        Assert.Contains(diagnostics, diagnostic =>
            diagnostic.Severity == AudioBankDiagnosticSeverity.Warning &&
            diagnostic.Field == "soundFiles[0].loop");
    }

    [Fact]
    public void ValidateAudioBank_AcceptsLoopingWavWithForwardLoop()
    {
        var root = CreateTempProjectRoot(CreateWave(new LoopSpec(Type: 0, Start: 2, End: 5)));
        var bank = AudioBankYamlParser.Parse("""
            AudioBank:
              soundFiles:
                - enumName: LOOPING
                  filename: looping
                  loop: true
            """);

        var diagnostics = AudioBankValidator.Validate(bank, root);

        Assert.DoesNotContain(diagnostics, diagnostic => diagnostic.Field == "soundFiles[0].loop");
    }

    private static string CreateTempProjectRoot(byte[] wav)
    {
        var root = Path.Combine(Path.GetTempPath(), "usagi-audio-" + Guid.NewGuid().ToString("N"));
        var audioDir = Path.Combine(root, "Data", "Audio");
        Directory.CreateDirectory(audioDir);
        File.WriteAllBytes(Path.Combine(audioDir, "looping.wav"), wav);
        return root;
    }

    private static byte[] CreateWave(LoopSpec? loop)
    {
        using var stream = new MemoryStream();
        WriteAscii(stream, "RIFF");
        WriteUInt32(stream, 0);
        WriteAscii(stream, "WAVE");

        WriteAscii(stream, "fmt ");
        WriteUInt32(stream, 16);
        WriteUInt16(stream, 1);
        WriteUInt16(stream, 2);
        WriteUInt32(stream, 48_000);
        WriteUInt32(stream, 192_000);
        WriteUInt16(stream, 4);
        WriteUInt16(stream, 16);

        if (loop is not null)
        {
            WriteAscii(stream, "smpl");
            WriteUInt32(stream, 60);
            WriteUInt32(stream, 0); // manufacturer
            WriteUInt32(stream, 0); // product
            WriteUInt32(stream, 0); // sample period
            WriteUInt32(stream, 0); // root MIDI note
            WriteUInt32(stream, 0); // pitch fraction
            WriteUInt32(stream, 0); // SMPTE format
            WriteUInt32(stream, 0); // SMPTE offset
            WriteUInt32(stream, 1); // loop count
            WriteUInt32(stream, 0); // sampler data
            WriteUInt32(stream, 0); // cue point
            WriteUInt32(stream, loop.Value.Type);
            WriteUInt32(stream, loop.Value.Start);
            WriteUInt32(stream, loop.Value.End);
            WriteUInt32(stream, 0); // fraction
            WriteUInt32(stream, 0); // play count
        }

        WriteAscii(stream, "data");
        WriteUInt32(stream, 16);
        stream.Write(new byte[16]);

        var bytes = stream.ToArray();
        BinaryPrimitives.WriteUInt32LittleEndian(bytes.AsSpan(4, 4), (uint)(bytes.Length - 8));
        return bytes;
    }

    private static void WriteAscii(Stream stream, string tag)
    {
        foreach (var character in tag)
        {
            stream.WriteByte((byte)character);
        }
    }

    private static void WriteUInt16(Stream stream, ushort value)
    {
        Span<byte> bytes = stackalloc byte[2];
        BinaryPrimitives.WriteUInt16LittleEndian(bytes, value);
        stream.Write(bytes);
    }

    private static void WriteUInt32(Stream stream, uint value)
    {
        Span<byte> bytes = stackalloc byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(bytes, value);
        stream.Write(bytes);
    }

    private readonly record struct LoopSpec(uint Type, uint Start, uint End);
}
