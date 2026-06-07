using System.Globalization;
using YamlDotNet.RepresentationModel;

namespace Usagi.ToolCore.Entities;

public sealed class EntityHierarchyLoader
{
    private static readonly HashSet<string> ReservedKeys = new(StringComparer.Ordinal)
    {
        "Children",
        "Inherits",
        "Overrides",
        "InitializerEvents"
    };

    private readonly IReadOnlyList<string> _includeDirectories;

    public EntityHierarchyLoader(IEnumerable<string>? includeDirectories = null)
    {
        _includeDirectories = includeDirectories?.Select(Path.GetFullPath).ToArray() ?? [];
    }

    public EntityDocument LoadFile(string path)
    {
        var fullPath = Path.GetFullPath(path);
        var text = File.ReadAllText(fullPath);
        return Load(fullPath, text);
    }

    public EntityDocument Load(string sourcePath, string text)
    {
        var diagnostics = new List<string>();
        var root = ParseRoot(sourcePath, text, diagnostics);
        var rootName = Path.GetFileNameWithoutExtension(sourcePath);
        var rootNode = BuildEntity(rootName, root, diagnostics);

        var includes = new Dictionary<string, EntityNode>(StringComparer.Ordinal);
        ResolveIncludes(rootNode, sourcePath, diagnostics, includes, []);

        return new EntityDocument(sourcePath, rootNode, diagnostics, includes);
    }

    private static YamlMappingNode ParseRoot(string sourcePath, string text, List<string> diagnostics)
    {
        if (text.Contains("<%", StringComparison.Ordinal))
        {
            diagnostics.Add("Template expressions are present; the entity loader parses source YAML without evaluating ERB.");
        }

        var yaml = new YamlStream();
        using var reader = new StringReader(text);
        yaml.Load(reader);

        if (yaml.Documents.Count == 0 || yaml.Documents[0].RootNode is not YamlMappingNode root)
        {
            throw new InvalidDataException($"{sourcePath} must contain a YAML mapping at the document root.");
        }

        return root;
    }

    private static EntityNode BuildEntity(string fallbackName, YamlMappingNode node, List<string> diagnostics)
    {
        var name = GetIdentifierName(node) ?? fallbackName;
        var inherits = ReadStringList(node, "Inherits", diagnostics);
        var children = ReadChildren(node, diagnostics);
        var components = ReadComponents(node);
        var overrides = ReadOverrides(node, diagnostics);
        var initializerEvents = ReadInitializerEvents(node, diagnostics);

        return new EntityNode(name, components, children, inherits, overrides, initializerEvents);
    }

    private static IReadOnlyList<EntityComponent> ReadComponents(YamlMappingNode node)
    {
        return node.Children
            .Select(pair => (Key: ScalarValue(pair.Key), Value: pair.Value))
            .Where(pair => pair.Key is not null && !ReservedKeys.Contains(pair.Key))
            .Select(pair => new EntityComponent(pair.Key!, Summarize(pair.Value), MappingFields(pair.Value)))
            .ToArray();
    }

    private static IReadOnlyList<EntityNode> ReadChildren(YamlMappingNode node, List<string> diagnostics)
    {
        var childrenNode = GetNode(node, "Children");
        if (childrenNode is null || IsEmptyScalar(childrenNode))
        {
            return Array.Empty<EntityNode>();
        }

        if (childrenNode is YamlSequenceNode sequence)
        {
            var children = new List<EntityNode>();
            for (var i = 0; i < sequence.Children.Count; i++)
            {
                if (sequence.Children[i] is YamlMappingNode child)
                {
                    children.Add(BuildEntity($"Child{i}", child, diagnostics));
                }
                else
                {
                    diagnostics.Add($"Children[{i}] is not a YAML mapping.");
                }
            }

            return children;
        }

        if (childrenNode is YamlMappingNode mapping)
        {
            var children = new List<EntityNode>();
            foreach (var (key, value) in mapping.Children)
            {
                if (value is YamlMappingNode child)
                {
                    children.Add(BuildEntity(ScalarValue(key) ?? "Child", child, diagnostics));
                }
                else
                {
                    diagnostics.Add($"Child '{ScalarValue(key) ?? "<unknown>"}' is not a YAML mapping.");
                }
            }

            return children;
        }

        diagnostics.Add("Children is present but is not a sequence or mapping.");
        return Array.Empty<EntityNode>();
    }

    private static IReadOnlyList<EntityOverride> ReadOverrides(YamlMappingNode node, List<string> diagnostics)
    {
        var overridesNode = GetNode(node, "Overrides");
        if (overridesNode is null || IsEmptyScalar(overridesNode))
        {
            return Array.Empty<EntityOverride>();
        }

        if (overridesNode is not YamlSequenceNode sequence)
        {
            diagnostics.Add("Overrides is present but is not a sequence.");
            return Array.Empty<EntityOverride>();
        }

        var overrides = new List<EntityOverride>();
        for (var i = 0; i < sequence.Children.Count; i++)
        {
            if (sequence.Children[i] is not YamlMappingNode mapping)
            {
                diagnostics.Add($"Overrides[{i}] is not a YAML mapping.");
                continue;
            }

            var targetEntityId = GetNode(mapping, "EntityWithID") is { } target ? ScalarValue(target) : null;
            if (string.IsNullOrWhiteSpace(targetEntityId))
            {
                diagnostics.Add($"Overrides[{i}] is missing EntityWithID.");
                continue;
            }

            var componentOverrides = mapping.Children
                .Select(pair => (Key: ScalarValue(pair.Key), Value: pair.Value))
                .Where(pair => pair.Key is not null && pair.Key != "EntityWithID")
                .ToDictionary(
                    pair => pair.Key!,
                    pair => (IReadOnlyDictionary<string, object?>)MappingFields(pair.Value),
                    StringComparer.Ordinal);

            overrides.Add(new EntityOverride(targetEntityId, componentOverrides));
        }

        return overrides;
    }

    private static IReadOnlyList<IReadOnlyDictionary<string, object?>> ReadInitializerEvents(
        YamlMappingNode node,
        List<string> diagnostics)
    {
        var eventsNode = GetNode(node, "InitializerEvents");
        if (eventsNode is null || IsEmptyScalar(eventsNode))
        {
            return Array.Empty<IReadOnlyDictionary<string, object?>>();
        }

        if (eventsNode is not YamlSequenceNode sequence)
        {
            diagnostics.Add("InitializerEvents is present but is not a sequence.");
            return Array.Empty<IReadOnlyDictionary<string, object?>>();
        }

        var events = new List<IReadOnlyDictionary<string, object?>>();
        for (var i = 0; i < sequence.Children.Count; i++)
        {
            if (sequence.Children[i] is YamlMappingNode mapping)
            {
                events.Add(MappingFields(mapping));
            }
            else
            {
                diagnostics.Add($"InitializerEvents[{i}] is not a YAML mapping.");
            }
        }

        return events;
    }

    private static IReadOnlyList<string> ReadStringList(YamlMappingNode node, string key, List<string> diagnostics)
    {
        var value = GetNode(node, key);
        if (value is null || IsEmptyScalar(value))
        {
            return Array.Empty<string>();
        }

        if (value is YamlSequenceNode sequence)
        {
            var values = new List<string>();
            for (var i = 0; i < sequence.Children.Count; i++)
            {
                var item = ScalarValue(sequence.Children[i]);
                if (string.IsNullOrWhiteSpace(item))
                {
                    diagnostics.Add($"{key}[{i}] is not a scalar value.");
                }
                else
                {
                    values.Add(item);
                }
            }

            return values;
        }

        var scalar = ScalarValue(value);
        if (string.IsNullOrWhiteSpace(scalar))
        {
            diagnostics.Add($"{key} is present but is not a scalar or sequence.");
            return Array.Empty<string>();
        }

        return [scalar];
    }

    private void ResolveIncludes(
        EntityNode node,
        string sourcePath,
        List<string> diagnostics,
        Dictionary<string, EntityNode> includes,
        HashSet<string> active)
    {
        foreach (var inheritedName in node.Inherits)
        {
            if (active.Contains(inheritedName))
            {
                diagnostics.Add($"Recursive entity inheritance detected for '{inheritedName}'.");
                continue;
            }

            if (includes.ContainsKey(inheritedName))
            {
                continue;
            }

            var includePath = FindInclude(sourcePath, inheritedName);
            if (includePath is null)
            {
                diagnostics.Add($"Inherited entity '{inheritedName}' was not found in the include paths.");
                continue;
            }

            active.Add(inheritedName);
            var includeText = File.ReadAllText(includePath);
            var includeRoot = ParseRoot(includePath, includeText, diagnostics);
            var includeNode = BuildEntity(inheritedName, includeRoot, diagnostics);
            includes[inheritedName] = includeNode;
            ResolveIncludes(includeNode, includePath, diagnostics, includes, active);
            active.Remove(inheritedName);
        }

        foreach (var child in node.Children)
        {
            ResolveIncludes(child, sourcePath, diagnostics, includes, active);
        }
    }

    private string? FindInclude(string sourcePath, string inheritedName)
    {
        foreach (var directory in CandidateIncludeDirectories(sourcePath))
        {
            var path = Path.Combine(directory, inheritedName + ".yml");
            if (File.Exists(path))
            {
                return path;
            }
        }

        return null;
    }

    private IEnumerable<string> CandidateIncludeDirectories(string sourcePath)
    {
        var sourceDirectory = Path.GetDirectoryName(Path.GetFullPath(sourcePath));
        if (!string.IsNullOrEmpty(sourceDirectory))
        {
            yield return sourceDirectory;
        }

        foreach (var directory in _includeDirectories)
        {
            yield return directory;
        }
    }

    private static string? GetIdentifierName(YamlMappingNode node)
    {
        if (GetNode(node, "Identifier") is not YamlMappingNode identifier)
        {
            return null;
        }

        return GetNode(identifier, "name") is { } name ? ScalarValue(name) : null;
    }

    private static YamlNode? GetNode(YamlMappingNode node, string key)
    {
        foreach (var pair in node.Children)
        {
            if (ScalarValue(pair.Key) == key)
            {
                return pair.Value;
            }
        }

        return null;
    }

    private static Dictionary<string, object?> MappingFields(YamlNode node)
    {
        if (node is not YamlMappingNode mapping)
        {
            return new Dictionary<string, object?>(StringComparer.Ordinal);
        }

        return mapping.Children
            .Select(pair => (Key: ScalarValue(pair.Key), Value: ParseValue(pair.Value)))
            .Where(pair => pair.Key is not null)
            .ToDictionary(pair => pair.Key!, pair => pair.Value, StringComparer.Ordinal);
    }

    private static object? ParseValue(YamlNode node)
    {
        return node switch
        {
            YamlMappingNode mapping => MappingFields(mapping),
            YamlSequenceNode sequence => sequence.Children.Select(ParseValue).ToList(),
            YamlScalarNode scalar => ParseScalar(scalar.Value),
            _ => null
        };
    }

    private static object? ParseScalar(string? value)
    {
        if (string.IsNullOrWhiteSpace(value))
        {
            return null;
        }

        if (bool.TryParse(value, out var boolValue))
        {
            return boolValue;
        }

        if (long.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var integerValue))
        {
            return integerValue;
        }

        if (double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var doubleValue))
        {
            return doubleValue;
        }

        return value;
    }

    private static string Summarize(YamlNode node)
    {
        return node switch
        {
            YamlMappingNode mapping => $"{mapping.Children.Count} fields",
            YamlSequenceNode sequence => $"{sequence.Children.Count} items",
            YamlScalarNode scalar when string.IsNullOrWhiteSpace(scalar.Value) => "present",
            YamlScalarNode scalar => scalar.Value ?? "present",
            _ => node.NodeType.ToString()
        };
    }

    private static string? ScalarValue(YamlNode node)
    {
        return node is YamlScalarNode scalar ? scalar.Value : null;
    }

    private static bool IsEmptyScalar(YamlNode node)
    {
        return node is YamlScalarNode scalar && string.IsNullOrWhiteSpace(scalar.Value);
    }
}
