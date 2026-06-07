namespace Usagi.ToolCore.Entities;

public sealed class EntityEditorSession
{
    private readonly IReadOnlyList<string> _includeDirectories;

    private EntityEditorSession(
        string sourcePath,
        IReadOnlyList<string> includeDirectories,
        EditableEntityDocument document,
        IReadOnlyDictionary<string, EntityNode> includes)
    {
        SourcePath = sourcePath;
        _includeDirectories = includeDirectories;
        Document = document;
        Includes = includes;
    }

    public string SourcePath { get; private set; }
    public EditableEntityDocument Document { get; private set; }
    public IReadOnlyDictionary<string, EntityNode> Includes { get; private set; }
    public IReadOnlyList<string> Diagnostics => Document.Diagnostics;
    public bool IsDirty => Document.IsDirty;

    public static EntityEditorSession Open(string path, IEnumerable<string>? includeDirectories = null)
    {
        var fullPath = Path.GetFullPath(path);
        var includes = includeDirectories?.Select(Path.GetFullPath).ToArray() ?? [];
        var loader = new EntityHierarchyLoader(includes);
        var document = loader.LoadFile(fullPath);
        return new EntityEditorSession(
            fullPath,
            includes,
            EditableEntityDocument.FromReadOnly(document),
            document.Includes);
    }

    public void Reload()
    {
        var loader = new EntityHierarchyLoader(_includeDirectories);
        var document = loader.LoadFile(SourcePath);
        Document = EditableEntityDocument.FromReadOnly(document);
        Includes = document.Includes;
    }

    public IReadOnlyList<EntityOutlineItem> GetOutline()
    {
        var items = new List<EntityOutlineItem>();
        AddOutlineItem(items, Document.Root, "", Document.Root.DisplayName, depth: 0);
        return items;
    }

    public EditableEntityNode GetEntity(string path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return Document.Root;
        }

        var current = Document.Root;
        foreach (var part in path.Split('/', StringSplitOptions.RemoveEmptyEntries))
        {
            if (!int.TryParse(part, out var index) || index < 0 || index >= current.Children.Count)
            {
                throw new ArgumentOutOfRangeException(nameof(path), path, "Entity path does not resolve to an entity.");
            }

            current = current.Children[index];
        }

        return current;
    }

    public IReadOnlyList<EntityComponentView> GetComponents(string path)
    {
        return GetEntity(path)
            .Components
            .Select(component => new EntityComponentView(component.Name, component.Fields.Count, component.Fields))
            .ToArray();
    }

    public IReadOnlyList<EntityIncludeView> GetIncludeViews()
    {
        return Includes
            .OrderBy(pair => pair.Key, StringComparer.Ordinal)
            .Select(pair => new EntityIncludeView(
                pair.Key,
                pair.Value.DisplayName,
                pair.Value.Components.Select(component => component.Name).ToArray()))
            .ToArray();
    }

    public void RenameEntity(string path, string newName)
    {
        Document.Rename(GetEntity(path), newName);
    }

    public void AddComponent(string path, string componentName, Dictionary<string, object?>? fields = null)
    {
        Document.AddComponent(GetEntity(path), componentName, fields);
    }

    public void SetComponentField(string path, string componentName, string fieldName, object? value)
    {
        var component = GetEntity(path).Components.FirstOrDefault(component => component.Name == componentName);
        if (component is null)
        {
            throw new InvalidOperationException($"Entity '{path}' does not have component '{componentName}'.");
        }

        Document.SetComponentField(component, fieldName, value);
    }

    public string Save()
    {
        Document.Save();
        return SourcePath;
    }

    public string SaveAs(string path)
    {
        var fullPath = Path.GetFullPath(path);
        Document.SaveAs(fullPath);
        SourcePath = fullPath;
        Document = new EditableEntityDocument(fullPath, Document.Root, Document.Diagnostics);
        Document.History.MarkSaved();
        return SourcePath;
    }

    private static void AddOutlineItem(
        List<EntityOutlineItem> items,
        EditableEntityNode entity,
        string path,
        string displayPath,
        int depth)
    {
        items.Add(new EntityOutlineItem(
            path,
            displayPath,
            entity.DisplayName,
            depth,
            entity.Components.Select(component => component.Name).ToArray(),
            entity.Inherits.ToArray()));

        for (var i = 0; i < entity.Children.Count; i++)
        {
            var childPath = string.IsNullOrEmpty(path) ? i.ToString() : $"{path}/{i}";
            AddOutlineItem(items, entity.Children[i], childPath, $"{displayPath}/{entity.Children[i].DisplayName}", depth + 1);
        }
    }
}

public sealed record EntityOutlineItem(
    string Path,
    string DisplayPath,
    string DisplayName,
    int Depth,
    IReadOnlyList<string> Components,
    IReadOnlyList<string> Inherits);

public sealed record EntityComponentView(
    string Name,
    int FieldCount,
    IReadOnlyDictionary<string, object?> Fields);

public sealed record EntityIncludeView(
    string Name,
    string DisplayName,
    IReadOnlyList<string> Components);
