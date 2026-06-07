using Usagi.ToolCore.Commands;

namespace Usagi.ToolCore.Entities;

public sealed class EditableEntityDocument
{
    private readonly EntityYamlWriter _writer = new();

    public string SourcePath { get; }
    public EditableEntityNode Root { get; }
    public CommandHistory History { get; } = new();
    public IReadOnlyList<string> Diagnostics { get; }

    public bool IsDirty => History.IsDirty;

    public event Action? Changed;

    public EditableEntityDocument(string sourcePath, EditableEntityNode root, IReadOnlyList<string>? diagnostics = null)
    {
        SourcePath = sourcePath;
        Root = root;
        Diagnostics = diagnostics ?? [];
        History.Changed += () => Changed?.Invoke();
    }

    public static EditableEntityDocument FromReadOnly(EntityDocument document)
    {
        var editableRoot = EditableEntityNode.FromReadOnly(document.Root);
        return new EditableEntityDocument(document.SourcePath, editableRoot, document.Diagnostics);
    }

    public static EditableEntityDocument Create(string sourcePath, string entityName)
    {
        var root = new EditableEntityNode(entityName);
        root.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = entityName });
        return new EditableEntityDocument(sourcePath, root);
    }

    public void Rename(EditableEntityNode entity, string newName)
    {
        History.Execute(new RenameEntityCommand(entity, newName));
    }

    public EditableEntityNode AddChild(EditableEntityNode parent, string childName)
    {
        var command = new AddChildCommand(parent, childName);
        History.Execute(command);
        return parent.Children[^1];
    }

    public void RemoveChild(EditableEntityNode parent, EditableEntityNode child)
    {
        History.Execute(new RemoveChildCommand(parent, child));
    }

    public void AddComponent(EditableEntityNode entity, string componentName, Dictionary<string, object?>? fields = null)
    {
        History.Execute(new AddComponentCommand(entity, componentName, fields));
    }

    public void RemoveComponent(EditableEntityNode entity, string componentName)
    {
        History.Execute(new RemoveComponentCommand(entity, componentName));
    }

    public string ToYaml()
    {
        return _writer.Write(Root);
    }

    public void Save()
    {
        var yaml = ToYaml();
        File.WriteAllText(SourcePath, yaml);
        History.MarkSaved();
    }

    public void SaveAs(string path)
    {
        var yaml = ToYaml();
        File.WriteAllText(path, yaml);
    }
}
