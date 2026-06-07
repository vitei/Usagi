namespace Usagi.ToolCore.Entities;

public sealed record EntityComponent(
    string Name,
    string Summary,
    IReadOnlyDictionary<string, object?> Fields)
{
    public EntityComponent(string name, string summary)
        : this(name, summary, new Dictionary<string, object?>())
    {
    }
}
