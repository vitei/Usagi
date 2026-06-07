using Usagi.ToolCore.Process;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class ProcessRunnerTests
{
    private readonly ProcessRunner _runner = new();

    [Fact]
    public void RunCapturesStandardOutput()
    {
        var result = RunShell("echo hello");

        Assert.Equal(0, result.ExitCode);
        Assert.True(result.Success);
        Assert.Contains("hello", result.StandardOutput, StringComparison.Ordinal);
    }

    [Fact]
    public void RunCapturesExitCode()
    {
        var result = RunShell(OperatingSystem.IsWindows() ? "exit /b 42" : "exit 42");

        Assert.Equal(42, result.ExitCode);
        Assert.False(result.Success);
    }

    [Fact]
    public void RunCapturesStandardError()
    {
        var result = RunShell("echo error 1>&2");

        Assert.Contains("error", result.StandardError, StringComparison.Ordinal);
    }

    [Fact]
    public void RunPreservesSourceAsset()
    {
        var result = RunShell("echo test", sourceAsset: "test.yml");

        Assert.Equal("test.yml", result.SourceAsset);
    }

    [Fact]
    public async Task RunAsyncCapturesOutput()
    {
        var (executable, arguments) = ShellCommand("echo async");

        var result = await _runner.RunAsync(executable, arguments);

        Assert.True(result.Success);
        Assert.Contains("async", result.StandardOutput, StringComparison.Ordinal);
    }

    [Fact]
    public void AllLinesAggregatesOutputAndError()
    {
        var result = RunShell("echo out && echo err 1>&2");

        Assert.Contains(result.AllLines, line => line.Contains("out", StringComparison.Ordinal));
        Assert.Contains(result.AllLines, line => line.Contains("err", StringComparison.Ordinal));
    }

    private ProcessResult RunShell(string command, string? sourceAsset = null)
    {
        var (executable, arguments) = ShellCommand(command);
        return _runner.Run(executable, arguments, sourceAsset: sourceAsset);
    }

    private static (string Executable, string[] Arguments) ShellCommand(string command)
    {
        return OperatingSystem.IsWindows()
            ? ("cmd.exe", ["/c", command])
            : ("/bin/sh", ["-c", command]);
    }
}
