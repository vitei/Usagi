namespace Usagi.ToolCore.Entities;

public sealed record EntityDocument(
    string SourcePath,
    EntityNode Root,
    IReadOnlyList<string> Diagnostics,
    IReadOnlyDictionary<string, EntityNode> Includes)
{
    public EntityDocument(string sourcePath, EntityNode root, IReadOnlyList<string> diagnostics)
        : this(sourcePath, root, diagnostics, new Dictionary<string, EntityNode>(StringComparer.Ordinal))
    {
    }
}
