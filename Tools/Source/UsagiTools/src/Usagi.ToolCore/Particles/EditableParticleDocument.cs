// Usagi.ToolCore - Particle Editor Support
// Document wrapper for editable particles with command history

using Usagi.ToolCore.Commands;

namespace Usagi.ToolCore.Particles;

/// <summary>
/// Document wrapper for an editable emitter with undo/redo support.
/// </summary>
public sealed class EditableEmitterDocument
{
    public EditableEmitter Emitter { get; }
    public CommandHistory History { get; } = new();
    public bool IsDirty => History.IsDirty;

    public EditableEmitterDocument(EditableEmitter emitter)
    {
        Emitter = emitter;
    }

    /// <summary>
    /// Loads an emitter document from a YAML file.
    /// </summary>
    public static EditableEmitterDocument Load(string path)
    {
        var emitter = ParticleYamlParser.ParseEmitter(path);
        return new EditableEmitterDocument(emitter);
    }

    /// <summary>
    /// Creates a new emitter document with default settings.
    /// </summary>
    public static EditableEmitterDocument CreateNew(string name = "NewEmitter")
    {
        var emitter = EditableEmitter.CreateDefault(name);
        return new EditableEmitterDocument(emitter);
    }

    /// <summary>
    /// Saves the emitter to its source path.
    /// </summary>
    public void Save()
    {
        if (string.IsNullOrEmpty(Emitter.SourcePath))
            throw new InvalidOperationException("No source path set. Use SaveAs instead.");

        ParticleYamlWriter.SaveEmitter(Emitter, Emitter.SourcePath);
        History.MarkSaved();
    }

    /// <summary>
    /// Saves the emitter to a new path.
    /// </summary>
    public void SaveAs(string path)
    {
        Emitter.SourcePath = path;
        Emitter.Name = Path.GetFileNameWithoutExtension(path);
        Save();
    }

    // Command wrappers for common operations

    public void SetEmissionRate(float value)
    {
        if (Emitter.Emission.EmissionRate.Frames.Count > 0)
        {
            var oldValue = Emitter.Emission.EmissionRate.Frames[0].Value;
            History.Execute(new ActionCommand(
                () => Emitter.Emission.EmissionRate.Frames[0].Value = value,
                () => Emitter.Emission.EmissionRate.Frames[0].Value = oldValue,
                $"Set emission rate to {value}"));
        }
    }

    public void SetMaxParticles(int value)
    {
        var oldValue = Emitter.Emission.MaxParticles;
        History.Execute(new ActionCommand(
            () => Emitter.Emission.MaxParticles = value,
            () => Emitter.Emission.MaxParticles = oldValue,
            $"Set max particles to {value}"));
    }

    public void SetLifespan(float value)
    {
        if (Emitter.Life.Frames.Count > 0)
        {
            var oldValue = Emitter.Life.Frames[0].Value;
            History.Execute(new ActionCommand(
                () => Emitter.Life.Frames[0].Value = value,
                () => Emitter.Life.Frames[0].Value = oldValue,
                $"Set lifespan to {value}"));
        }
    }

    public void SetShape(EmitterShape shape)
    {
        var oldShape = Emitter.Shape;
        History.Execute(new ActionCommand(
            () => Emitter.Shape = shape,
            () => Emitter.Shape = oldShape,
            $"Set shape to {shape}"));
    }

    public void SetTexture(int slot, string textureName)
    {
        while (Emitter.Textures.Count <= slot)
        {
            Emitter.Textures.Add(new TextureSlot());
        }

        var oldName = Emitter.Textures[slot].Name;
        History.Execute(new ActionCommand(
            () => Emitter.Textures[slot].Name = textureName,
            () => Emitter.Textures[slot].Name = oldName,
            $"Set texture {slot} to {textureName}"));
    }

    public void AddTexture(string textureName)
    {
        var slot = new TextureSlot { Name = textureName };
        History.Execute(new ActionCommand(
            () => Emitter.Textures.Add(slot),
            () => Emitter.Textures.Remove(slot),
            $"Add texture {textureName}"));
    }

    public void RemoveTexture(int slot)
    {
        if (slot < 0 || slot >= Emitter.Textures.Count) return;

        var texture = Emitter.Textures[slot];
        History.Execute(new ActionCommand(
            () => Emitter.Textures.RemoveAt(slot),
            () => Emitter.Textures.Insert(slot, texture),
            $"Remove texture {texture.Name}"));
    }
}

/// <summary>
/// Document wrapper for an editable effect with undo/redo support.
/// </summary>
public sealed class EditableEffectDocument
{
    public EditableEffect Effect { get; }
    public CommandHistory History { get; } = new();
    public bool IsDirty => History.IsDirty;

    public EditableEffectDocument(EditableEffect effect)
    {
        Effect = effect;
    }

    /// <summary>
    /// Loads an effect document from a YAML file.
    /// </summary>
    public static EditableEffectDocument Load(string path)
    {
        var effect = ParticleYamlParser.ParseEffect(path);
        return new EditableEffectDocument(effect);
    }

    /// <summary>
    /// Creates a new effect document with a single emitter reference.
    /// </summary>
    public static EditableEffectDocument CreateNew(string name, string emitterName)
    {
        var effect = EditableEffect.CreateDefault(name, emitterName);
        return new EditableEffectDocument(effect);
    }

    /// <summary>
    /// Saves the effect to its source path.
    /// </summary>
    public void Save()
    {
        if (string.IsNullOrEmpty(Effect.SourcePath))
            throw new InvalidOperationException("No source path set. Use SaveAs instead.");

        ParticleYamlWriter.SaveEffect(Effect, Effect.SourcePath);
        History.MarkSaved();
    }

    /// <summary>
    /// Saves the effect to a new path.
    /// </summary>
    public void SaveAs(string path)
    {
        Effect.SourcePath = path;
        Effect.Name = Path.GetFileNameWithoutExtension(path);
        Save();
    }

    // Command wrappers for common operations

    public EmitterInstance AddEmitter(string emitterName)
    {
        var instance = EmitterInstance.Create(emitterName);
        History.Execute(new ActionCommand(
            () => Effect.Emitters.Add(instance),
            () => Effect.Emitters.Remove(instance),
            $"Add emitter {emitterName}"));
        return instance;
    }

    public void RemoveEmitter(EmitterInstance instance)
    {
        var index = Effect.Emitters.IndexOf(instance);
        if (index < 0) return;

        History.Execute(new ActionCommand(
            () => Effect.Emitters.Remove(instance),
            () => Effect.Emitters.Insert(index, instance),
            $"Remove emitter {instance.EmitterName}"));
    }

    public void SetEmitterPosition(EmitterInstance instance, Vec3 position)
    {
        var oldPos = instance.Position;
        History.Execute(new ActionCommand(
            () => instance.Position = position,
            () => instance.Position = oldPos,
            $"Set {instance.EmitterName} position"));
    }

    public void SetEmitterScale(EmitterInstance instance, Vec3 scale)
    {
        var oldScale = instance.Scale;
        History.Execute(new ActionCommand(
            () => instance.Scale = scale,
            () => instance.Scale = oldScale,
            $"Set {instance.EmitterName} scale"));
    }

    public void SetEmitterRotation(EmitterInstance instance, Vec3 rotation)
    {
        var oldRot = instance.Rotation;
        History.Execute(new ActionCommand(
            () => instance.Rotation = rotation,
            () => instance.Rotation = oldRot,
            $"Set {instance.EmitterName} rotation"));
    }

    public void SetPreloadCount(int count)
    {
        var oldCount = Effect.PreloadCount;
        History.Execute(new ActionCommand(
            () => Effect.PreloadCount = count,
            () => Effect.PreloadCount = oldCount,
            $"Set preload count to {count}"));
    }
}

/// <summary>
/// Simple action-based command for particle property changes.
/// </summary>
internal sealed class ActionCommand : ICommand
{
    private readonly Action _execute;
    private readonly Action _undo;
    private readonly string _description;

    public ActionCommand(Action execute, Action undo, string description)
    {
        _execute = execute;
        _undo = undo;
        _description = description;
    }

    public string Description => _description;
    public void Execute() => _execute();
    public void Undo() => _undo();
    public override string ToString() => _description;
}
