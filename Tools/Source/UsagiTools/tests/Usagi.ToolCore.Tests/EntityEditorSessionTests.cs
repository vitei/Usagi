using Usagi.ToolCore.Entities;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class EntityEditorSessionTests
{
    [Fact]
    public void OpenBuildsOutlineComponentsAndIncludeViews()
    {
        using var fixture = EntityFixture.Create();

        var session = EntityEditorSession.Open(fixture.RootEntityPath, [fixture.IncludeDirectory]);

        Assert.Empty(session.Diagnostics);
        Assert.False(session.IsDirty);

        var outline = session.GetOutline();
        Assert.Equal(["", "0"], outline.Select(item => item.Path));
        Assert.Equal("Enemy/WeaponSocket", outline[1].DisplayPath);
        Assert.Equal(["Identifier", "ModelComponent"], outline[0].Components);

        var components = session.GetComponents("");
        Assert.Contains(components, component =>
            component.Name == "ModelComponent" &&
            component.Fields["name"]?.ToString() == "Models/enemy.vmdf");

        var include = Assert.Single(session.GetIncludeViews());
        Assert.Equal("BaseEnemy", include.Name);
        Assert.Contains("HealthComponent", include.Components);
    }

    [Fact]
    public void EditsUseCommandHistoryAndSaveRoundTrips()
    {
        using var fixture = EntityFixture.Create();
        var session = EntityEditorSession.Open(fixture.RootEntityPath, [fixture.IncludeDirectory]);

        session.RenameEntity("", "EliteEnemy");
        session.SetComponentField("", "ModelComponent", "name", "Models/elite.vmdf");
        session.AddComponent("0", "TransformComponent", new Dictionary<string, object?> { ["x"] = 1.0 });

        Assert.True(session.IsDirty);
        Assert.Equal("EliteEnemy", session.GetEntity("").DisplayName);
        Assert.Contains(session.GetComponents("0"), component => component.Name == "TransformComponent");

        session.Document.History.Undo();
        Assert.DoesNotContain(session.GetComponents("0"), component => component.Name == "TransformComponent");
        session.Document.History.Redo();

        session.Save();
        Assert.False(session.IsDirty);

        var reloaded = EntityEditorSession.Open(fixture.RootEntityPath, [fixture.IncludeDirectory]);
        Assert.Equal("EliteEnemy", reloaded.GetEntity("").DisplayName);
        Assert.Contains(reloaded.GetComponents(""), component =>
            component.Name == "ModelComponent" &&
            component.Fields["name"]?.ToString() == "Models/elite.vmdf");
        Assert.Contains(reloaded.GetComponents("0"), component => component.Name == "TransformComponent");
    }

    [Fact]
    public void MissingEntityPathIsReported()
    {
        using var fixture = EntityFixture.Create();
        var session = EntityEditorSession.Open(fixture.RootEntityPath, [fixture.IncludeDirectory]);

        var error = Assert.Throws<ArgumentOutOfRangeException>(() => session.GetEntity("2"));

        Assert.Equal("path", error.ParamName);
    }

    private sealed class EntityFixture : IDisposable
    {
        private readonly DirectoryInfo _root;

        private EntityFixture(DirectoryInfo root, string includeDirectory, string rootEntityPath)
        {
            _root = root;
            IncludeDirectory = includeDirectory;
            RootEntityPath = rootEntityPath;
        }

        public string IncludeDirectory { get; }
        public string RootEntityPath { get; }

        public static EntityFixture Create()
        {
            var root = Directory.CreateTempSubdirectory("usagi-entity-session-");
            var includeDirectory = Directory.CreateDirectory(Path.Combine(root.FullName, "Includes"));
            File.WriteAllText(Path.Combine(includeDirectory.FullName, "BaseEnemy.yml"), """
Identifier:
  name: BaseEnemy
HealthComponent:
  fLife: 10.0
""");

            var rootEntityPath = Path.Combine(root.FullName, "Enemy.yml");
            File.WriteAllText(rootEntityPath, """
Inherits:
  - BaseEnemy
Identifier:
  name: Enemy
ModelComponent:
  name: Models/enemy.vmdf
Children:
  - Identifier:
      name: WeaponSocket
    AttachmentComponent:
      bone: hand_r
""");

            return new EntityFixture(root, includeDirectory.FullName, rootEntityPath);
        }

        public void Dispose()
        {
            _root.Delete(recursive: true);
        }
    }
}
