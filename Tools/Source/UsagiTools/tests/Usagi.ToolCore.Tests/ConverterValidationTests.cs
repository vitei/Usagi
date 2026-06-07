using Usagi.ToolCore.Process;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class ConverterValidationTests
{
    [Fact]
    public void ConverterRunnerPreservesSourceAssetOnFailure()
    {
        var runner = CreateShellBackedRubyRunner();

        var result = runner.RunScript(ShellSwitch, ["echo converter-error 1>&2 && " + ShellExit(7)], sourceAsset: "Data/test.yml");

        Assert.Equal(7, result.ExitCode);
        Assert.False(result.Success);
        Assert.Equal("Data/test.yml", result.SourceAsset);
        Assert.Contains("converter-error", result.StandardError, StringComparison.Ordinal);
    }

    [Fact]
    public void ConverterRunnerCapturesValidationOutput()
    {
        var runner = CreateShellBackedRubyRunner();

        var result = runner.RunScript(ShellSwitch, ["echo converter-ok"]);

        Assert.True(result.Success);
        Assert.Contains("converter-ok", result.StandardOutput, StringComparison.Ordinal);
    }

    private static string ShellSwitch => OperatingSystem.IsWindows() ? "/c" : "-c";

    private static RubyRunner CreateShellBackedRubyRunner()
    {
        return new RubyRunner(OperatingSystem.IsWindows() ? "cmd.exe" : "/bin/sh");
    }

    private static string ShellExit(int exitCode)
    {
        return OperatingSystem.IsWindows() ? $"exit /b {exitCode}" : $"exit {exitCode}";
    }
}
