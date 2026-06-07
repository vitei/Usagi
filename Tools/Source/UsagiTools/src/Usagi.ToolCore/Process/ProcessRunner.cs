using System.Diagnostics;
using System.Text;

namespace Usagi.ToolCore.Process;

public sealed record ProcessResult(
    int ExitCode,
    string StandardOutput,
    string StandardError,
    string? SourceAsset = null)
{
    public bool Success => ExitCode == 0;

    public IEnumerable<string> AllLines =>
        SplitLines(StandardOutput).Concat(SplitLines(StandardError));

    private static IEnumerable<string> SplitLines(string value) =>
        value.Replace("\r\n", "\n", StringComparison.Ordinal)
            .Split('\n', StringSplitOptions.RemoveEmptyEntries)
            .Select(line => line.TrimEnd('\r'));
}

public sealed class ProcessRunner
{
    public async Task<ProcessResult> RunAsync(
        string executable,
        IEnumerable<string> arguments,
        string? workingDirectory = null,
        string? sourceAsset = null,
        CancellationToken cancellationToken = default)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = executable,
            WorkingDirectory = workingDirectory ?? Environment.CurrentDirectory,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };

        foreach (var arg in arguments)
        {
            startInfo.ArgumentList.Add(arg);
        }

        var stdout = new StringBuilder();
        var stderr = new StringBuilder();

        using var process = new System.Diagnostics.Process { StartInfo = startInfo };

        process.OutputDataReceived += (_, e) =>
        {
            if (e.Data is not null)
            {
                stdout.AppendLine(e.Data);
            }
        };

        process.ErrorDataReceived += (_, e) =>
        {
            if (e.Data is not null)
            {
                stderr.AppendLine(e.Data);
            }
        };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        try
        {
            await process.WaitForExitAsync(cancellationToken);
        }
        catch (OperationCanceledException)
        {
            if (!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
                await process.WaitForExitAsync(CancellationToken.None);
            }

            throw;
        }

        return new ProcessResult(
            process.ExitCode,
            stdout.ToString(),
            stderr.ToString(),
            sourceAsset);
    }

    public ProcessResult Run(
        string executable,
        IEnumerable<string> arguments,
        string? workingDirectory = null,
        string? sourceAsset = null,
        int timeoutMs = 30000)
    {
        using var cancellation = new CancellationTokenSource(timeoutMs);

        try
        {
            return RunAsync(executable, arguments, workingDirectory, sourceAsset, cancellation.Token)
                .GetAwaiter()
                .GetResult();
        }
        catch (OperationCanceledException)
        {
            return new ProcessResult(-1, string.Empty, "Process timed out.", sourceAsset);
        }
    }
}
