using Usagi.ToolCore.Settings;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class WorkspaceSettingsTests
{
    [Fact]
    public void SaveToAndLoadFromRoundTripSettings()
    {
        using var temp = new TemporaryDirectory();
        var settingsPath = Path.Combine(temp.Path, "workspace.json");
        var settings = new WorkspaceSettings
        {
            LastProjectPath = "fixtures/usagi-project",
            RecentProjects = ["fixtures/usagi-project"],
            RubyExecutable = "ruby"
        };

        settings.SaveTo(settingsPath);
        var loaded = WorkspaceSettings.LoadFrom(settingsPath);

        Assert.Equal(settings.LastProjectPath, loaded.LastProjectPath);
        Assert.Equal(settings.RecentProjects, loaded.RecentProjects);
        Assert.Equal(settings.RubyExecutable, loaded.RubyExecutable);
    }

    [Fact]
    public void LoadFromReturnsDefaultSettingsForInvalidJson()
    {
        using var temp = new TemporaryDirectory();
        var settingsPath = Path.Combine(temp.Path, "workspace.json");
        File.WriteAllText(settingsPath, "{ invalid json");

        var settings = WorkspaceSettings.LoadFrom(settingsPath);

        Assert.Null(settings.LastProjectPath);
        Assert.Empty(settings.RecentProjects);
    }

    [Fact]
    public void AddRecentProjectMovesExistingProjectToFrontAndCapsList()
    {
        var settings = new WorkspaceSettings();
        for (var i = 0; i < 12; i++)
        {
            settings.AddRecentProject($"Project{i}");
        }

        settings.AddRecentProject("Project5");

        Assert.Equal("Project5", settings.LastProjectPath);
        Assert.Equal("Project5", settings.RecentProjects[0]);
        Assert.Equal(10, settings.RecentProjects.Count);
        Assert.Equal(1, settings.RecentProjects.Count(project => project == "Project5"));
    }

    private sealed class TemporaryDirectory : IDisposable
    {
        public string Path { get; } = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "usagi_settings_" + Guid.NewGuid().ToString("N"));

        public TemporaryDirectory()
        {
            Directory.CreateDirectory(Path);
        }

        public void Dispose()
        {
            Directory.Delete(Path, recursive: true);
        }
    }
}
