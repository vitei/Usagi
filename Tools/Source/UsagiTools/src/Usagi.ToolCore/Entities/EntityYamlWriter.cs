using System.Collections;
using System.Globalization;
using System.Text;

namespace Usagi.ToolCore.Entities;

public sealed class EntityYamlWriter
{
    private readonly StringBuilder _output = new();
    private int _indent;

    public string Write(EditableEntityNode entity)
    {
        _output.Clear();
        _indent = 0;

        WriteEntityBody(entity);

        return _output.ToString();
    }

    private void WriteEntityBody(EditableEntityNode entity)
    {
        if (entity.Inherits.Count > 0)
        {
            WriteLine("Inherits:");
            _indent++;
            foreach (var inherit in entity.Inherits)
            {
                WriteLine($"- {FormatString(inherit)}");
            }

            _indent--;
        }

        foreach (var component in entity.Components)
        {
            WriteComponent(component);
        }

        if (entity.Overrides.Count > 0)
        {
            WriteLine("Overrides:");
            _indent++;
            foreach (var entityOverride in entity.Overrides)
            {
                WriteLine($"- EntityWithID: {FormatString(entityOverride.TargetEntityId)}");
                _indent++;
                foreach (var (componentName, fields) in entityOverride.ComponentOverrides)
                {
                    WriteFieldsBlock(componentName, fields);
                }

                _indent--;
            }

            _indent--;
        }

        if (entity.InitializerEvents.Count > 0)
        {
            WriteLine("InitializerEvents:");
            _indent++;
            foreach (var initializerEvent in entity.InitializerEvents)
            {
                WriteSequenceMapping(initializerEvent);
            }

            _indent--;
        }

        if (entity.Children.Count > 0)
        {
            WriteLine("Children:");
            _indent++;
            foreach (var child in entity.Children)
            {
                WriteChild(child);
            }

            _indent--;
        }
    }

    private void WriteChild(EditableEntityNode child)
    {
        if (child.Components.Count == 0 && child.Inherits.Count == 0)
        {
            WriteLine("- {}");
            return;
        }

        var first = true;

        if (child.Inherits.Count > 0)
        {
            WriteChildTopLevel("Inherits:", first);
            first = false;
            _indent += 2;
            foreach (var inherit in child.Inherits)
            {
                WriteLine($"- {FormatString(inherit)}");
            }

            _indent -= 2;
        }

        foreach (var component in child.Components)
        {
            WriteChildTopLevel($"{component.Name}:", first);
            first = false;
            if (component.Fields.Count > 0)
            {
                _indent += 2;
                foreach (var (key, value) in component.Fields)
                {
                    WriteField(key, value);
                }

                _indent -= 2;
            }
        }

        if (child.Children.Count > 0)
        {
            WriteChildTopLevel("Children:", first);
            _indent += 2;
            foreach (var grandchild in child.Children)
            {
                WriteChild(grandchild);
            }

            _indent -= 2;
        }
    }

    private void WriteChildTopLevel(string line, bool first)
    {
        _output.Append(Indent());
        _output.Append(first ? "- " : "  ");
        _output.AppendLine(line);
    }

    private void WriteComponent(EditableComponent component)
    {
        if (component.Fields.Count == 0)
        {
            WriteLine($"{component.Name}:");
            return;
        }

        WriteFieldsBlock(component.Name, component.Fields);
    }

    private void WriteFieldsBlock(string key, IReadOnlyDictionary<string, object?> fields)
    {
        WriteLine($"{key}:");
        _indent++;
        foreach (var (fieldName, value) in fields)
        {
            WriteField(fieldName, value);
        }

        _indent--;
    }

    private void WriteField(string key, object? value)
    {
        if (value is IReadOnlyDictionary<string, object?> mapping)
        {
            WriteLine($"{key}:");
            _indent++;
            foreach (var (nestedKey, nestedValue) in mapping)
            {
                WriteField(nestedKey, nestedValue);
            }

            _indent--;
            return;
        }

        if (IsList(value, out var list))
        {
            WriteLine($"{key}:");
            _indent++;
            foreach (var item in list)
            {
                if (item is IReadOnlyDictionary<string, object?> itemMapping)
                {
                    WriteSequenceMapping(itemMapping);
                }
                else
                {
                    WriteLine($"- {FormatValue(item)}");
                }
            }

            _indent--;
            return;
        }

        WriteLine($"{key}: {FormatValue(value)}");
    }

    private void WriteSequenceMapping(IReadOnlyDictionary<string, object?> mapping)
    {
        if (mapping.Count == 0)
        {
            WriteLine("- {}");
            return;
        }

        var first = true;
        var sequenceIndent = _indent;
        foreach (var (key, value) in mapping)
        {
            if (first)
            {
                _output.Append(Indent());
                _output.Append("- ");
                _output.AppendLine($"{key}: {InlineOrEmpty(value)}");
                first = false;
            }
            else
            {
                _indent = sequenceIndent + 1;
                WriteLine($"{key}: {InlineOrEmpty(value)}");
            }

            if (value is IReadOnlyDictionary<string, object?> nested)
            {
                _indent = sequenceIndent + 2;
                foreach (var (nestedKey, nestedValue) in nested)
                {
                    WriteField(nestedKey, nestedValue);
                }
            }
        }

        _indent = sequenceIndent;
    }

    private static string InlineOrEmpty(object? value)
    {
        return value is IReadOnlyDictionary<string, object?> ? string.Empty : FormatValue(value);
    }

    private static bool IsList(object? value, out IEnumerable list)
    {
        if (value is not null && value is not string && value is IEnumerable enumerable)
        {
            list = enumerable;
            return true;
        }

        list = Array.Empty<object>();
        return false;
    }

    private static string FormatValue(object? value)
    {
        return value switch
        {
            null => string.Empty,
            string s => FormatString(s),
            bool b => b ? "true" : "false",
            IFormattable formattable => formattable.ToString(null, CultureInfo.InvariantCulture),
            _ => value.ToString() ?? string.Empty
        };
    }

    private static string FormatString(string value)
    {
        if (value.Length == 0 ||
            value.Contains('\n') ||
            value.Contains(':') ||
            value.Contains('#') ||
            value.Contains('"') ||
            value.StartsWith(' ') ||
            value.EndsWith(' ') ||
            value is "true" or "false" or "null" or "~" ||
            double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out _))
        {
            return $"\"{value.Replace("\"", "\\\"", StringComparison.Ordinal)}\"";
        }

        return value;
    }

    private void WriteLine(string line)
    {
        _output.Append(Indent());
        _output.AppendLine(line);
    }

    private string Indent() => new(' ', _indent * 2);
}
