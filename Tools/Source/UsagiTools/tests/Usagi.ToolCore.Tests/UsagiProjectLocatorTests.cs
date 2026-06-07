using Usagi.ToolCore.Projects;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class UsagiProjectLocatorTests
{
    [Fact]
    public void FromPathReturnsProjectForValidRoot()
    {
        using var temp = CreateProjectRoot();

        var project = UsagiProjectLocator.FromPath(temp.Path);

        Assert.NotNull(project);
        Assert.Equal(Path.Combine(temp.Path, "Data"), project.DataPath);
        Assert.True(project.HasValidStructure);
    }

    [Fact]
    public void FromPathRejectsIncompleteRoot()
    {
        using var temp = new TemporaryDirectory("usagi_incomplete_");
        Directory.CreateDirectory(Path.Combine(temp.Path, "Tools"));

        var project = UsagiProjectLocator.FromPath(temp.Path);

        Assert.Null(project);
    }

    [Fact]
    public void TryLocateWalksUpFromChildDirectory()
    {
        using var temp = CreateProjectRoot();
        var child = Path.Combine(temp.Path, "Data", "Levels", "Example");
        Directory.CreateDirectory(child);

        var project = UsagiProjectLocator.TryLocate(child);

        Assert.NotNull(project);
        Assert.Equal(temp.Path, project.RootPath);
    }

    private static TemporaryDirectory CreateProjectRoot()
    {
        var temp = new TemporaryDirectory("usagi_project_");
        Directory.CreateDirectory(Path.Combine(temp.Path, "Data"));
        Directory.CreateDirectory(Path.Combine(temp.Path, "Engine"));
        Directory.CreateDirectory(Path.Combine(temp.Path, "Tools"));
        return temp;
    }

    private sealed class TemporaryDirectory : IDisposable
    {
        public string Path { get; }

        public TemporaryDirectory(string prefix)
        {
            Path = System.IO.Path.Combine(System.IO.Path.GetTempPath(), prefix + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(Path);
        }

        public void Dispose()
        {
            Directory.Delete(Path, recursive: true);
        }
    }
}
