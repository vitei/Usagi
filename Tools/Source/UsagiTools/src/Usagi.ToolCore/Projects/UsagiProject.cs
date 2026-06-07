namespace Usagi.ToolCore.Projects;

public sealed record UsagiProject(string RootPath)
{
    public string DataPath => Path.Combine(RootPath, "Data");
    public string EnginePath => Path.Combine(RootPath, "Engine");
    public string ToolsPath => Path.Combine(RootPath, "Tools");
    public string RubyPath => Path.Combine(ToolsPath, "ruby");
    public string BuildPath => Path.Combine(RootPath, "_build");
    public string RomfilesWinPath => Path.Combine(RootPath, "_romfiles", "win");

    public string ProcessHierarchyScript => Path.Combine(RubyPath, "process_hierarchy.rb");

    public bool HasValidStructure =>
        Directory.Exists(EnginePath) &&
        Directory.Exists(DataPath) &&
        Directory.Exists(ToolsPath);
}
