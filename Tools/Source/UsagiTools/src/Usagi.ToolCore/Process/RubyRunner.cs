namespace Usagi.ToolCore.Process;

public sealed class RubyRunner
{
    private readonly ProcessRunner _processRunner = new();
    private readonly string _rubyExecutable;

    public RubyRunner(string? rubyExecutable = null)
    {
        _rubyExecutable = rubyExecutable ?? FindRuby();
    }

    public string RubyExecutable => _rubyExecutable;

    public async Task<ProcessResult> RunScriptAsync(
        string scriptPath,
        IEnumerable<string> arguments,
        string? workingDirectory = null,
        string? sourceAsset = null,
        CancellationToken cancellationToken = default)
    {
        var args = new List<string> { scriptPath };
        args.AddRange(arguments);

        return await _processRunner.RunAsync(
            _rubyExecutable,
            args,
            workingDirectory,
            sourceAsset,
            cancellationToken);
    }

    public ProcessResult RunScript(
        string scriptPath,
        IEnumerable<string> arguments,
        string? workingDirectory = null,
        string? sourceAsset = null,
        int timeoutMs = 30000)
    {
        var args = new List<string> { scriptPath };
        args.AddRange(arguments);

        return _processRunner.Run(
            _rubyExecutable,
            args,
            workingDirectory,
            sourceAsset,
            timeoutMs);
    }

    private static string FindRuby()
    {
        var pathDirs = Environment.GetEnvironmentVariable("PATH")?.Split(Path.PathSeparator) ?? [];

        foreach (var dir in pathDirs)
        {
            var rubyPath = Path.Combine(dir, "ruby.exe");
            if (File.Exists(rubyPath))
            {
                return rubyPath;
            }

            rubyPath = Path.Combine(dir, "ruby");
            if (File.Exists(rubyPath))
            {
                return rubyPath;
            }
        }

        return "ruby";
    }
}
