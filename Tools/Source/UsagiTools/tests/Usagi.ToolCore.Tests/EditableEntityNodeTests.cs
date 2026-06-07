using Usagi.ToolCore.Entities;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class EditableEntityNodeTests
{
    [Fact]
    public void AddChildCreatesEntityWithIdentifier()
    {
        var root = new EditableEntityNode("Root");

        var child = root.AddChild("ChildEntity");

        Assert.Single(root.Children);
        Assert.Equal("ChildEntity", child.DisplayName);
        Assert.Contains(child.Components, c => c.Name == "Identifier" && c.Fields["name"]?.ToString() == "ChildEntity");
    }

    [Fact]
    public void RemoveChildRemovesEntity()
    {
        var root = new EditableEntityNode("Root");
        var child = root.AddChild("ChildEntity");

        root.RemoveChild(child);

        Assert.Empty(root.Children);
    }

    [Fact]
    public void AddComponentAddsNewComponent()
    {
        var entity = new EditableEntityNode("Entity");

        entity.AddComponent("TransformComponent", new Dictionary<string, object?>
        {
            ["position"] = new Dictionary<string, object?> { ["x"] = 1.0, ["y"] = 2.0, ["z"] = 3.0 }
        });

        Assert.Single(entity.Components);
        Assert.Equal("TransformComponent", entity.Components[0].Name);
    }

    [Fact]
    public void RemoveComponentRemovesExistingComponent()
    {
        var entity = new EditableEntityNode("Entity");
        entity.AddComponent("TransformComponent");
        entity.AddComponent("ModelComponent");

        var result = entity.RemoveComponent("TransformComponent");

        Assert.True(result);
        Assert.Single(entity.Components);
        Assert.Equal("ModelComponent", entity.Components[0].Name);
    }

    [Fact]
    public void RemoveComponentReturnsFalseForMissingComponent()
    {
        var entity = new EditableEntityNode("Entity");

        var result = entity.RemoveComponent("NonExistent");

        Assert.False(result);
    }

    [Fact]
    public void RenameUpdatesDisplayNameAndIdentifier()
    {
        var entity = new EditableEntityNode("OldName");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "OldName" });

        entity.Rename("NewName");

        Assert.Equal("NewName", entity.DisplayName);
        Assert.Equal("NewName", entity.Components[0].Fields["name"]);
    }

    [Fact]
    public void RenameCreatesIdentifierIfMissing()
    {
        var entity = new EditableEntityNode("OldName");

        entity.Rename("NewName");

        Assert.Equal("NewName", entity.DisplayName);
        Assert.Single(entity.Components);
        Assert.Equal("Identifier", entity.Components[0].Name);
        Assert.Equal("NewName", entity.Components[0].Fields["name"]);
    }

    [Fact]
    public void FromReadOnlyPreservesStructure()
    {
        var readOnly = new EntityNode(
            "TestEntity",
            [new EntityComponent("Identifier", "name field"), new EntityComponent("TransformComponent", "3 fields")],
            [new EntityNode("ChildA", [new EntityComponent("ModelComponent", "1 field")], [], ["BaseModel"], 0, 0)],
            ["BaseEntity"],
            1,
            2);

        var editable = EditableEntityNode.FromReadOnly(readOnly);

        Assert.Equal("TestEntity", editable.DisplayName);
        Assert.Equal(2, editable.Components.Count);
        Assert.Single(editable.Children);
        Assert.Equal("ChildA", editable.Children[0].DisplayName);
        Assert.Single(editable.Inherits);
        Assert.Equal("BaseEntity", editable.Inherits[0]);
    }

    [Fact]
    public void FromReadOnlyPreservesComponentFields()
    {
        var readOnly = new EntityNode(
            "Camera",
            [new EntityComponent("CameraComponent", "1 field", new Dictionary<string, object?> { ["fFOV"] = 45.0 })],
            [],
            [],
            0,
            0);

        var editable = EditableEntityNode.FromReadOnly(readOnly);

        Assert.Equal(45.0, editable.Components[0].Fields["fFOV"]);
    }

    [Fact]
    public void EditableDocumentCommandsUndoAndRedoEntityEdits()
    {
        var document = EditableEntityDocument.Create("Test.yml", "Root");

        var child = document.AddChild(document.Root, "Child");
        document.AddComponent(child, "TransformComponent");
        document.History.Undo();
        document.History.Redo();

        Assert.Single(document.Root.Children);
        Assert.Contains(document.Root.Children[0].Components, component => component.Name == "TransformComponent");

        document.RemoveChild(document.Root, child);
        Assert.Empty(document.Root.Children);

        document.History.Undo();
        Assert.Same(child, document.Root.Children[0]);
    }
}
