using Usagi.ToolCore.Commands;

namespace Usagi.ToolCore.Entities;

internal sealed class RenameEntityCommand : ICommand
{
    private readonly EditableEntityNode _entity;
    private readonly string _oldName;
    private readonly string _newName;

    public RenameEntityCommand(EditableEntityNode entity, string newName)
    {
        _entity = entity;
        _oldName = entity.DisplayName;
        _newName = newName;
    }

    public string Description => $"Rename to {_newName}";

    public void Execute() => _entity.Rename(_newName);

    public void Undo() => _entity.Rename(_oldName);
}

internal sealed class AddChildCommand : ICommand
{
    private readonly EditableEntityNode _parent;
    private readonly string _childName;
    private EditableEntityNode? _addedChild;
    private int _index;

    public AddChildCommand(EditableEntityNode parent, string childName)
    {
        _parent = parent;
        _childName = childName;
        _index = parent.Children.Count;
    }

    public string Description => $"Add child {_childName}";

    public void Execute()
    {
        if (_addedChild is null)
        {
            _addedChild = new EditableEntityNode(_childName);
            _addedChild.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = _childName });
        }

        var insertAt = Math.Clamp(_index, 0, _parent.Children.Count);
        _parent.Children.Insert(insertAt, _addedChild);
    }

    public void Undo()
    {
        if (_addedChild is not null)
        {
            _parent.RemoveChild(_addedChild);
        }
    }
}

internal sealed class RemoveChildCommand : ICommand
{
    private readonly EditableEntityNode _parent;
    private readonly EditableEntityNode _child;
    private int _index = -1;

    public RemoveChildCommand(EditableEntityNode parent, EditableEntityNode child)
    {
        _parent = parent;
        _child = child;
    }

    public string Description => $"Remove {_child.DisplayName}";

    public void Execute()
    {
        _index = _parent.Children.IndexOf(_child);
        if (_index >= 0)
        {
            _parent.RemoveChild(_child);
        }
    }

    public void Undo()
    {
        if (_index < 0)
        {
            return;
        }

        var insertAt = Math.Clamp(_index, 0, _parent.Children.Count);
        _parent.Children.Insert(insertAt, _child);
    }
}

internal sealed class AddComponentCommand : ICommand
{
    private readonly EditableEntityNode _entity;
    private readonly string _componentName;
    private readonly Dictionary<string, object?>? _fields;
    private EditableComponent? _addedComponent;
    private int _index;

    public AddComponentCommand(EditableEntityNode entity, string componentName, Dictionary<string, object?>? fields = null)
    {
        _entity = entity;
        _componentName = componentName;
        _fields = fields;
        _index = entity.Components.Count;
    }

    public string Description => $"Add {_componentName}";

    public void Execute()
    {
        _addedComponent ??= new EditableComponent(_componentName, _fields);

        var insertAt = Math.Clamp(_index, 0, _entity.Components.Count);
        _entity.Components.Insert(insertAt, _addedComponent);
    }

    public void Undo()
    {
        if (_addedComponent is not null)
        {
            _entity.Components.Remove(_addedComponent);
        }
    }
}

internal sealed class RemoveComponentCommand : ICommand
{
    private readonly EditableEntityNode _entity;
    private readonly string _componentName;
    private EditableComponent? _removedComponent;
    private int _index = -1;

    public RemoveComponentCommand(EditableEntityNode entity, string componentName)
    {
        _entity = entity;
        _componentName = componentName;
    }

    public string Description => $"Remove {_componentName}";

    public void Execute()
    {
        _removedComponent = _entity.Components.FirstOrDefault(component => component.Name == _componentName);
        if (_removedComponent is null)
        {
            _index = -1;
            return;
        }

        _index = _entity.Components.IndexOf(_removedComponent);
        _entity.Components.Remove(_removedComponent);
    }

    public void Undo()
    {
        if (_removedComponent is null || _index < 0)
        {
            return;
        }

        var insertAt = Math.Clamp(_index, 0, _entity.Components.Count);
        _entity.Components.Insert(insertAt, _removedComponent);
    }
}
