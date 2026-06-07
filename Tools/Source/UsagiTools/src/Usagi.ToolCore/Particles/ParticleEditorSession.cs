namespace Usagi.ToolCore.Particles;

public sealed class ParticleEditorSession
{
    private ParticleEditorSession(
        EditableEmitterDocument? emitterDocument,
        EditableEffectDocument? effectDocument)
    {
        EmitterDocument = emitterDocument;
        EffectDocument = effectDocument;
    }

    public EditableEmitterDocument? EmitterDocument { get; private set; }
    public EditableEffectDocument? EffectDocument { get; private set; }

    public bool HasEmitter => EmitterDocument is not null;
    public bool HasEffect => EffectDocument is not null;
    public bool IsDirty => (EmitterDocument?.IsDirty ?? false) || (EffectDocument?.IsDirty ?? false);

    public static ParticleEditorSession OpenEmitter(string path)
    {
        return new ParticleEditorSession(EditableEmitterDocument.Load(Path.GetFullPath(path)), null);
    }

    public static ParticleEditorSession OpenEffect(string path)
    {
        return new ParticleEditorSession(null, EditableEffectDocument.Load(Path.GetFullPath(path)));
    }

    public static ParticleEditorSession OpenEffectWithEmitters(
        string effectPath,
        string emitterDirectory)
    {
        var effectDocument = EditableEffectDocument.Load(Path.GetFullPath(effectPath));
        var emitterPath = FindFirstEmitter(effectDocument.Effect, emitterDirectory);
        var emitterDocument = emitterPath is null ? null : EditableEmitterDocument.Load(emitterPath);
        return new ParticleEditorSession(emitterDocument, effectDocument);
    }

    public IReadOnlyList<ParticleTextureView> GetTextures()
    {
        var emitter = RequireEmitter();
        return emitter.Textures
            .Select((texture, index) => new ParticleTextureView(
                index,
                texture.Name,
                texture.PatternRepeatHor,
                texture.PatternRepeatVer,
                texture.Animation.Mode,
                texture.Animation.AnimIndex.ToArray(),
                texture.Animation.AnimTimeScale))
            .ToArray();
    }

    public IReadOnlyList<ParticleEmitterInstanceView> GetEmitterInstances()
    {
        var effect = RequireEffect();
        return effect.Emitters
            .Select((instance, index) => new ParticleEmitterInstanceView(
                index,
                instance.EmitterName,
                instance.Position,
                instance.Rotation,
                instance.Scale,
                instance.ParticleScale,
                instance.ReleaseFrame))
            .ToArray();
    }

    public ParticleEmitterSummary GetEmitterSummary()
    {
        var emitter = RequireEmitter();
        return new ParticleEmitterSummary(
            emitter.Name,
            emitter.SourcePath,
            emitter.Shape,
            emitter.Type,
            emitter.Emission.MaxParticles,
            emitter.Textures.Count,
            emitter.Emission.EmissionRate.Frames.FirstOrDefault()?.Value ?? 0.0f,
            emitter.Life.Frames.FirstOrDefault()?.Value ?? 0.0f);
    }

    public ParticleEffectSummary GetEffectSummary()
    {
        var effect = RequireEffect();
        return new ParticleEffectSummary(
            effect.Name,
            effect.SourcePath,
            effect.PreloadCount,
            effect.Emitters.Count);
    }

    public void SetMaxParticles(int value)
    {
        RequireEmitterDocument().SetMaxParticles(value);
    }

    public void SetEmitterTexture(int slot, string textureName)
    {
        RequireEmitterDocument().SetTexture(slot, textureName);
    }

    public void AddEmitterTexture(string textureName)
    {
        RequireEmitterDocument().AddTexture(textureName);
    }

    public EmitterInstance AddEffectEmitter(string emitterName)
    {
        return RequireEffectDocument().AddEmitter(emitterName);
    }

    public void SetEffectEmitterPosition(int index, Vec3 position)
    {
        var effectDocument = RequireEffectDocument();
        effectDocument.SetEmitterPosition(GetEmitterInstance(index), position);
    }

    public void SetPreloadCount(int count)
    {
        RequireEffectDocument().SetPreloadCount(count);
    }

    public void SaveAll()
    {
        EmitterDocument?.Save();
        EffectDocument?.Save();
    }

    public ParticleConversionPlan GetEmitterConversionPlan(string projectRoot, string? outputPath = null)
    {
        var emitter = RequireEmitter();
        return CreateConversionPlan(projectRoot, emitter.SourcePath, outputPath, ".pem");
    }

    public ParticleConversionPlan GetEffectConversionPlan(string projectRoot, string? outputPath = null)
    {
        var effect = RequireEffect();
        return CreateConversionPlan(projectRoot, effect.SourcePath, outputPath, ".pfx");
    }

    private EditableEmitterDocument RequireEmitterDocument() =>
        EmitterDocument ?? throw new InvalidOperationException("No emitter document is open.");

    private EditableEffectDocument RequireEffectDocument() =>
        EffectDocument ?? throw new InvalidOperationException("No effect document is open.");

    private EditableEmitter RequireEmitter() => RequireEmitterDocument().Emitter;

    private EditableEffect RequireEffect() => RequireEffectDocument().Effect;

    private EmitterInstance GetEmitterInstance(int index)
    {
        var effect = RequireEffect();
        if (index < 0 || index >= effect.Emitters.Count)
        {
            throw new ArgumentOutOfRangeException(nameof(index), index, "Emitter instance index is out of range.");
        }

        return effect.Emitters[index];
    }

    private static string? FindFirstEmitter(EditableEffect effect, string emitterDirectory)
    {
        foreach (var instance in effect.Emitters)
        {
            var path = Path.Combine(emitterDirectory, instance.EmitterName + ".yml");
            if (File.Exists(path))
            {
                return Path.GetFullPath(path);
            }
        }

        return null;
    }

    private static ParticleConversionPlan CreateConversionPlan(
        string projectRoot,
        string sourcePath,
        string? outputPath,
        string extension)
    {
        var fullRoot = Path.GetFullPath(projectRoot);
        var fullSource = Path.GetFullPath(sourcePath);
        var script = Path.Combine(fullRoot, "Tools", "ruby", "yml2vpb.rb");
        var particleSubdir = extension == ".pem" ? "Emitters" : "Effects";
        var resolvedOutput = outputPath is null
            ? Path.Combine(fullRoot, "_romfiles", "win", "Particle", particleSubdir, Path.GetFileNameWithoutExtension(sourcePath) + extension)
            : Path.GetFullPath(outputPath);

        return new ParticleConversionPlan(
            fullSource,
            resolvedOutput,
            script,
            [
                "-i",
                fullSource,
                "-o",
                resolvedOutput
            ]);
    }
}

public sealed record ParticleEmitterSummary(
    string Name,
    string SourcePath,
    EmitterShape Shape,
    ParticleType Type,
    int MaxParticles,
    int TextureCount,
    float EmissionRate,
    float Lifespan);

public sealed record ParticleEffectSummary(
    string Name,
    string SourcePath,
    int PreloadCount,
    int EmitterCount);

public sealed record ParticleTextureView(
    int Slot,
    string Name,
    int PatternRepeatHor,
    int PatternRepeatVer,
    TextureAnimMode AnimationMode,
    IReadOnlyList<int> AnimationIndices,
    float AnimationTimeScale);

public sealed record ParticleEmitterInstanceView(
    int Index,
    string EmitterName,
    Vec3 Position,
    Vec3 Rotation,
    Vec3 Scale,
    float ParticleScale,
    float ReleaseFrame);

public sealed record ParticleConversionPlan(
    string SourcePath,
    string OutputPath,
    string ScriptPath,
    IReadOnlyList<string> Arguments);
