using Usagi.ToolCore.Entities;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class EntityYamlWriterTests
{
    private readonly EntityYamlWriter _writer = new();
    private readonly EntityHierarchyLoader _loader = new();

    [Fact]
    public void WriteProducesValidYamlForSimpleEntity()
    {
        var entity = new EditableEntityNode("TestEntity");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "TestEntity" });
        entity.AddComponent("TransformComponent");

        var yaml = _writer.Write(entity);

        Assert.Contains("Identifier:", yaml);
        Assert.Contains("name: TestEntity", yaml);
        Assert.Contains("TransformComponent:", yaml);
    }

    [Fact]
    public void WriteProducesValidYamlWithChildren()
    {
        var entity = new EditableEntityNode("Parent");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "Parent" });
        var child = entity.AddChild("ChildA");
        child.AddComponent("ModelComponent");

        var yaml = _writer.Write(entity);

        Assert.Contains("Children:", yaml);
        Assert.Contains("Identifier:", yaml);
        Assert.Contains("ModelComponent:", yaml);
    }

    [Fact]
    public void WriteProducesValidYamlWithInherits()
    {
        var entity = new EditableEntityNode("DerivedEntity");
        entity.Inherits.Add("BaseEntity");
        entity.Inherits.Add("AnotherBase");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "DerivedEntity" });

        var yaml = _writer.Write(entity);

        Assert.Contains("Inherits:", yaml);
        Assert.Contains("- BaseEntity", yaml);
        Assert.Contains("- AnotherBase", yaml);
    }

    [Fact]
    public void WrittenYamlCanBeReloadedByLoader()
    {
        var entity = new EditableEntityNode("RoundTrip");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "RoundTrip" });
        entity.AddComponent("CameraComponent", new Dictionary<string, object?> { ["fFOV"] = 50.0 });
        var child = entity.AddChild("SubEntity");
        child.AddComponent("TransformComponent");

        var yaml = _writer.Write(entity);
        var reloaded = _loader.Load("RoundTrip.yml", yaml);

        Assert.Equal("RoundTrip", reloaded.Root.DisplayName);
        Assert.Single(reloaded.Root.Children);
        Assert.Equal("SubEntity", reloaded.Root.Children[0].DisplayName);
        Assert.Contains(
            reloaded.Root.Components,
            component => component.Name == "CameraComponent" && Convert.ToDouble(component.Fields["fFOV"]) == 50.0);
    }

    [Fact]
    public void WriteHandlesNestedFields()
    {
        var entity = new EditableEntityNode("Entity");
        entity.AddComponent("TransformComponent", new Dictionary<string, object?>
        {
            ["position"] = new Dictionary<string, object?>
            {
                ["x"] = 1.0,
                ["y"] = 2.0,
                ["z"] = 3.0
            }
        });

        var yaml = _writer.Write(entity);

        Assert.Contains("TransformComponent:", yaml);
        Assert.Contains("position:", yaml);
        Assert.Contains("x: 1", yaml);
        Assert.Contains("y: 2", yaml);
        Assert.Contains("z: 3", yaml);
    }

    [Fact]
    public void WriteProducesValidYamlWithOverrides()
    {
        var entity = new EditableEntityNode("EntityWithOverrides");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "EntityWithOverrides" });

        var ovr = new EditableOverride("TargetChild");
        ovr.ComponentOverrides["TransformComponent"] = new Dictionary<string, object?>
        {
            ["position"] = new Dictionary<string, object?> { ["x"] = 10.0 }
        };
        entity.Overrides.Add(ovr);

        var yaml = _writer.Write(entity);

        Assert.Contains("Overrides:", yaml);
        Assert.Contains("EntityWithID: TargetChild", yaml);
        Assert.Contains("TransformComponent:", yaml);

        var reloaded = _loader.Load("Overrides.yml", yaml);
        Assert.Single(reloaded.Root.Overrides);
        Assert.Equal("TargetChild", reloaded.Root.Overrides[0].TargetEntityId);
    }

    [Fact]
    public void WriteProducesValidYamlWithInitializerEvents()
    {
        var entity = new EditableEntityNode("EntityWithEvents");
        entity.AddComponent("Identifier", new Dictionary<string, object?> { ["name"] = "EntityWithEvents" });

        entity.InitializerEvents.Add(new Dictionary<string, object?>
        {
            ["SpawnEvent"] = new Dictionary<string, object?> { ["count"] = 5 }
        });

        var yaml = _writer.Write(entity);

        Assert.Contains("InitializerEvents:", yaml);
        Assert.Contains("SpawnEvent:", yaml);
        Assert.Contains("count: 5", yaml);

        var reloaded = _loader.Load("Events.yml", yaml);
        Assert.Equal(1, reloaded.Root.InitializerEventCount);
    }

    [Fact]
    public void EditableDocumentRoundTripsLoadedEntity()
    {
        const string yaml = """
Identifier:
  name: RoundTripLoaded
CameraComponent:
  fFOV: 60.0
Children:
  - Identifier:
      name: Child
    TransformComponent:
      position:
        x: 1.0
        y: 2.0
        z: 3.0
""";

        var loaded = _loader.Load("RoundTripLoaded.yml", yaml);
        var editable = EditableEntityDocument.FromReadOnly(loaded);
        var written = editable.ToYaml();
        var reloaded = _loader.Load("RoundTripLoaded.yml", written);

        Assert.Equal("RoundTripLoaded", reloaded.Root.DisplayName);
        Assert.Equal(60.0, Convert.ToDouble(reloaded.Root.Components.Single(component => component.Name == "CameraComponent").Fields["fFOV"]));
        Assert.Single(reloaded.Root.Children);
        Assert.Contains(reloaded.Root.Children[0].Components, component => component.Name == "TransformComponent");
    }
}
