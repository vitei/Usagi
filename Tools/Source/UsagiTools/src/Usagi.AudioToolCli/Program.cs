using Usagi.ToolCore.Audio;

var options = AudioToolOptions.Parse(args);
if (options.ShowHelp)
{
    AudioToolOptions.PrintHelp(Console.Out);
    return 0;
}

if (options.Error is not null)
{
    Console.Error.WriteLine(options.Error);
    Console.Error.WriteLine("Try '--help' for more information.");
    return 2;
}

try
{
    if (string.IsNullOrWhiteSpace(options.InputPath))
    {
        throw new FileNotFoundException("No input file specified.");
    }

    if (!File.Exists(options.InputPath))
    {
        throw new FileNotFoundException("Input file not found.", options.InputPath);
    }

    if (!options.ValidateOnly && string.IsNullOrWhiteSpace(options.OutputPath))
    {
        throw new ArgumentException("Output file was not specified.");
    }

    var bank = AudioBankYamlParser.ParseFile(options.InputPath);
    var diagnostics = AudioBankValidator.Validate(bank, options.ProjectRoot);
    foreach (var diagnostic in diagnostics)
    {
        var writer = diagnostic.Severity == AudioBankDiagnosticSeverity.Error ? Console.Error : Console.Out;
        writer.WriteLine($"{diagnostic.Severity}: {diagnostic.Field}: {diagnostic.Message}");
    }

    if (options.ValidateOnly)
    {
        return diagnostics.Any(diagnostic => diagnostic.Severity == AudioBankDiagnosticSeverity.Error) ? 1 : 0;
    }

    if (diagnostics.Any(diagnostic => diagnostic.Severity == AudioBankDiagnosticSeverity.Error))
    {
        return 1;
    }

    var output = options.NormalizeYaml
        ? AudioBankYamlWriter.Write(bank)
        : options.Proto
            ? FsidBuilder.WriteProto(bank, options.EnumName)
            : FsidBuilder.WriteHeader(bank, options.IfndefName, options.EnumName);

    var outputDirectory = Path.GetDirectoryName(Path.GetFullPath(options.OutputPath));
    if (!string.IsNullOrEmpty(outputDirectory))
    {
        Directory.CreateDirectory(outputDirectory);
    }

    File.WriteAllText(options.OutputPath, output);
    return 0;
}
catch (Exception ex)
{
    Console.Error.WriteLine(ex.Message);
    return 1;
}

internal sealed record AudioToolOptions
{
    public string InputPath { get; private init; } = "";
    public string OutputPath { get; private init; } = "";
    public string EnumName { get; private init; } = "UsagiAudio";
    public string IfndefName { get; private init; } = "_CLR_TANK_FSID_";
    public string? ProjectRoot { get; private init; }
    public bool Proto { get; private init; }
    public bool NormalizeYaml { get; private init; }
    public bool ValidateOnly { get; private init; }
    public bool ShowHelp { get; private init; }
    public string? Error { get; private init; }

    public static AudioToolOptions Parse(string[] args)
    {
        var options = new AudioToolOptions();
        for (var i = 0; i < args.Length; i++)
        {
            var arg = args[i];
            switch (arg)
            {
                case "-h":
                case "--help":
                    options = options with { ShowHelp = true };
                    break;
                case "-p":
                case "--proto":
                    options = options with { Proto = true };
                    break;
                case "--normalize-yaml":
                    options = options with { NormalizeYaml = true };
                    break;
                case "--validate":
                    options = options with { ValidateOnly = true };
                    break;
                case "-i":
                case "--input":
                    options = options with { InputPath = ReadValue(args, ref i, arg) };
                    break;
                case "-o":
                case "--output":
                    options = options with { OutputPath = ReadValue(args, ref i, arg) };
                    break;
                case "-e":
                case "--enumName":
                    options = options with { EnumName = ReadValue(args, ref i, arg) };
                    break;
                case "-g":
                case "--ifndefName":
                    options = options with { IfndefName = ReadValue(args, ref i, arg) };
                    break;
                case "--project-root":
                    options = options with { ProjectRoot = ReadValue(args, ref i, arg) };
                    break;
                default:
                    if (TrySplitOption(arg, out var name, out var value))
                    {
                        options = name switch
                        {
                            "--input" => options with { InputPath = value },
                            "--output" => options with { OutputPath = value },
                            "--enumName" => options with { EnumName = value },
                            "--ifndefName" => options with { IfndefName = value },
                            "--project-root" => options with { ProjectRoot = value },
                            "-i" => options with { InputPath = value },
                            "-o" => options with { OutputPath = value },
                            "-e" => options with { EnumName = value },
                            "-g" => options with { IfndefName = value },
                            _ => options with { Error = $"Unknown option: {name}" }
                        };
                    }
                    else
                    {
                        options = options with { Error = $"Unknown option: {arg}" };
                    }
                    break;
            }
        }

        return options;
    }

    public static void PrintHelp(TextWriter writer)
    {
        writer.WriteLine("Usage: fsidbuilder [OPTIONS]");
        writer.WriteLine("Build a .fsid file from an AudioBank .yml file.");
        writer.WriteLine();
        writer.WriteLine("Options:");
        writer.WriteLine("  -i, --input=VALUE          audio bank (.yml) to load.");
        writer.WriteLine("  -o, --output=VALUE         output file to generate.");
        writer.WriteLine("  -e, --enumName=VALUE       name of enum count constant to use");
        writer.WriteLine("  -g, --ifndefName=VALUE     name of #ifndef guard to use");
        writer.WriteLine("  -p, --proto                generate a protocol buffer file instead of a C++");
        writer.WriteLine("                               header");
        writer.WriteLine("      --normalize-yaml       write normalized AudioBank YAML");
        writer.WriteLine("      --validate             validate input without writing output");
        writer.WriteLine("      --project-root=VALUE   project root for asset validation");
        writer.WriteLine("  -h, --help                 show this message and exit");
    }

    private static string ReadValue(string[] args, ref int index, string option)
    {
        if (index + 1 >= args.Length)
        {
            throw new ArgumentException($"Missing value for {option}.");
        }

        index++;
        return args[index];
    }

    private static bool TrySplitOption(string arg, out string name, out string value)
    {
        var equals = arg.IndexOf('=', StringComparison.Ordinal);
        if (equals < 0)
        {
            name = "";
            value = "";
            return false;
        }

        name = arg[..equals];
        value = arg[(equals + 1)..];
        return true;
    }
}
