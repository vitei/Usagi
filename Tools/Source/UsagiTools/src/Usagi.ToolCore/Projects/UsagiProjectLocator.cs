namespace Usagi.ToolCore.Projects;

public static class UsagiProjectLocator
{
    public static UsagiProject? TryLocate(string? startDirectory = null)
    {
        var environmentRoot = Environment.GetEnvironmentVariable("USAGI_REPO_DIR");
        if (!string.IsNullOrWhiteSpace(environmentRoot) && IsUsagiRoot(environmentRoot))
        {
            return new UsagiProject(Path.GetFullPath(environmentRoot));
        }

        var usagiDir = Environment.GetEnvironmentVariable("USAGI_DIR");
        if (!string.IsNullOrWhiteSpace(usagiDir) && IsUsagiRoot(usagiDir))
        {
            return new UsagiProject(Path.GetFullPath(usagiDir));
        }

        var current = new DirectoryInfo(startDirectory ?? Environment.CurrentDirectory);
        while (current is not null)
        {
            if (IsUsagiRoot(current.FullName))
            {
                return new UsagiProject(current.FullName);
            }

            current = current.Parent;
        }

        return null;
    }

    public static UsagiProject Locate(string? startDirectory = null)
    {
        return TryLocate(startDirectory)
            ?? throw new DirectoryNotFoundException(
                "Could not locate a Usagi checkout. Set USAGI_DIR or USAGI_REPO_DIR, or launch from inside the repo.");
    }

    public static UsagiProject? FromPath(string path)
    {
        if (!Directory.Exists(path))
        {
            return null;
        }

        var fullPath = Path.GetFullPath(path);
        return IsUsagiRoot(fullPath) ? new UsagiProject(fullPath) : null;
    }

    private static bool IsUsagiRoot(string path)
    {
        return Directory.Exists(Path.Combine(path, "Engine")) &&
               Directory.Exists(Path.Combine(path, "Data")) &&
               Directory.Exists(Path.Combine(path, "Tools"));
    }
}
