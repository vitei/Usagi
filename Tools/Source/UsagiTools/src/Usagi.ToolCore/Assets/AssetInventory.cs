namespace Usagi.ToolCore.Assets;

public sealed record AssetItem(string Name, string FullPath, string RelativePath)
{
    public override string ToString() => RelativePath;
}

public static class AssetInventory
{
    private static readonly string[] YamlExtensions = [".yml", ".yaml"];

    public static IReadOnlyList<AssetItem> EnumerateYamlAssets(string rootDirectory)
    {
        return EnumerateAssets(rootDirectory, YamlExtensions);
    }

    public static IReadOnlyList<AssetItem> EnumerateAssets(string rootDirectory, IReadOnlyCollection<string> extensions)
    {
        if (!Directory.Exists(rootDirectory))
        {
            return Array.Empty<AssetItem>();
        }

        var normalizedExtensions = extensions
            .Select(extension => extension.StartsWith(".", StringComparison.Ordinal) ? extension : "." + extension)
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        return Directory.EnumerateFiles(rootDirectory, "*", SearchOption.AllDirectories)
            .Where(path => normalizedExtensions.Contains(Path.GetExtension(path)))
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .Select(path =>
            {
                var relative = Path.GetRelativePath(rootDirectory, path);
                return new AssetItem(Path.GetFileNameWithoutExtension(path), path, relative);
            })
            .ToArray();
    }
}
