namespace Usagi.ToolCore.Entities;

public sealed class EntityNode
{
    public EntityNode(
        string displayName,
        IReadOnlyList<EntityComponent> components,
        IReadOnlyList<EntityNode> children,
        IReadOnlyList<string> inherits,
        int overrideCount,
        int initializerEventCount)
        : this(displayName, components, children, inherits, [], [], overrideCount, initializerEventCount)
    {
    }

    public EntityNode(
        string displayName,
        IReadOnlyList<EntityComponent> components,
        IReadOnlyList<EntityNode> children,
        IReadOnlyList<string> inherits,
        IReadOnlyList<EntityOverride> overrides,
        IReadOnlyList<IReadOnlyDictionary<string, object?>> initializerEvents,
        int? overrideCount = null,
        int? initializerEventCount = null)
    {
        DisplayName = displayName;
        Components = components;
        Children = children;
        Inherits = inherits;
        Overrides = overrides;
        InitializerEvents = initializerEvents;
        OverrideCount = overrideCount ?? overrides.Count;
        InitializerEventCount = initializerEventCount ?? initializerEvents.Count;
    }

    public string DisplayName { get; }
    public IReadOnlyList<EntityComponent> Components { get; }
    public IReadOnlyList<EntityNode> Children { get; }
    public IReadOnlyList<string> Inherits { get; }
    public IReadOnlyList<EntityOverride> Overrides { get; }
    public IReadOnlyList<IReadOnlyDictionary<string, object?>> InitializerEvents { get; }
    public int OverrideCount { get; }
    public int InitializerEventCount { get; }

    public string ComponentSummary =>
        Components.Count == 0 ? "No components" : string.Join(", ", Components.Select(component => component.Name));
}

public sealed record EntityOverride(
    string TargetEntityId,
    IReadOnlyDictionary<string, IReadOnlyDictionary<string, object?>> ComponentOverrides);
