using System.Text.Json;

namespace Usagi.ToolCore.Settings;

public sealed class WorkspaceSettings
{
    private static readonly string SettingsDirectory =
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "UsagiTools");

    private static readonly string SettingsPath =
        Path.Combine(SettingsDirectory, "workspace.json");

    public string? LastProjectPath { get; set; }
    public List<string> RecentProjects { get; set; } = [];
    public string? RubyExecutable { get; set; }

    public static WorkspaceSettings Load() => LoadFrom(SettingsPath);

    public static WorkspaceSettings LoadFrom(string settingsPath)
    {
        if (!File.Exists(settingsPath))
        {
            return new WorkspaceSettings();
        }

        try
        {
            var json = File.ReadAllText(settingsPath);
            var settings = JsonSerializer.Deserialize<WorkspaceSettings>(json) ?? new WorkspaceSettings();
            settings.RecentProjects ??= [];
            return settings;
        }
        catch
        {
            return new WorkspaceSettings();
        }
    }

    public void Save() => SaveTo(SettingsPath);

    public void SaveTo(string settingsPath)
    {
        var settingsDirectory = Path.GetDirectoryName(settingsPath);
        if (!string.IsNullOrEmpty(settingsDirectory))
        {
            Directory.CreateDirectory(settingsDirectory);
        }

        var options = new JsonSerializerOptions { WriteIndented = true };
        var json = JsonSerializer.Serialize(this, options);
        File.WriteAllText(settingsPath, json);
    }

    public void AddRecentProject(string projectPath)
    {
        RecentProjects.Remove(projectPath);
        RecentProjects.Insert(0, projectPath);

        if (RecentProjects.Count > 10)
        {
            RecentProjects.RemoveRange(10, RecentProjects.Count - 10);
        }

        LastProjectPath = projectPath;
    }
}
