using Usagi.ToolCore.Assets;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class AssetInventoryTests
{
    [Fact]
    public void EnumerateYamlAssetsReturnsRelativePathsInStableOrder()
    {
        using var temp = new TemporaryDirectory();
        Directory.CreateDirectory(Path.Combine(temp.Path, "Nested"));
        File.WriteAllText(Path.Combine(temp.Path, "z.yml"), "");
        File.WriteAllText(Path.Combine(temp.Path, "Nested", "a.yaml"), "");
        File.WriteAllText(Path.Combine(temp.Path, "ignored.txt"), "");

        var assets = AssetInventory.EnumerateYamlAssets(temp.Path);

        Assert.Equal(["Nested" + Path.DirectorySeparatorChar + "a.yaml", "z.yml"], assets.Select(asset => asset.RelativePath));
        Assert.Equal(["a", "z"], assets.Select(asset => asset.Name));
    }

    [Fact]
    public void EnumerateAssetsFiltersExtensionsCaseInsensitively()
    {
        using var temp = new TemporaryDirectory();
        File.WriteAllText(Path.Combine(temp.Path, "model.VPB"), "");
        File.WriteAllText(Path.Combine(temp.Path, "sound.wav"), "");

        var assets = AssetInventory.EnumerateAssets(temp.Path, ["vpb"]);

        Assert.Single(assets);
        Assert.Equal("model", assets[0].Name);
    }

    [Fact]
    public void EnumerateAssetsReturnsEmptyListForMissingDirectory()
    {
        var assets = AssetInventory.EnumerateAssets(Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N")), ["yml"]);

        Assert.Empty(assets);
    }

    private sealed class TemporaryDirectory : IDisposable
    {
        public string Path { get; } = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "usagi_assets_" + Guid.NewGuid().ToString("N"));

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
