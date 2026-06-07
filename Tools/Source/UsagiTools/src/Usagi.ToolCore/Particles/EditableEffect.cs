// Usagi.ToolCore - Particle Editor Support
// Editable effect model (groups of emitters)

namespace Usagi.ToolCore.Particles;

/// <summary>
/// A reference to an emitter within an effect group.
/// </summary>
public sealed class EmitterInstance
{
    /// <summary>
    /// The emitter name (references an emitter YAML file).
    /// </summary>
    public string EmitterName { get; set; } = "";

    /// <summary>
    /// Local scale of this emitter instance.
    /// </summary>
    public Vec3 Scale { get; set; } = Vec3.One;

    /// <summary>
    /// Local rotation (Euler angles in degrees).
    /// </summary>
    public Vec3 Rotation { get; set; } = Vec3.Zero;

    /// <summary>
    /// Local position offset.
    /// </summary>
    public Vec3 Position { get; set; } = Vec3.Zero;

    /// <summary>
    /// Additional particle scale multiplier.
    /// </summary>
    public float ParticleScale { get; set; } = 1.0f;

    /// <summary>
    /// Frame at which this emitter starts releasing particles.
    /// </summary>
    public float ReleaseFrame { get; set; } = 0.0f;

    /// <summary>
    /// Creates a new emitter instance referencing the given emitter.
    /// </summary>
    public static EmitterInstance Create(string emitterName)
    {
        return new EmitterInstance { EmitterName = emitterName };
    }
}

/// <summary>
/// A complete editable particle effect (group of emitters).
/// </summary>
public sealed class EditableEffect
{
    /// <summary>
    /// The effect name (derived from filename).
    /// </summary>
    public string Name { get; set; } = "NewEffect";

    /// <summary>
    /// Path to the source YAML file.
    /// </summary>
    public string SourcePath { get; set; } = "";

    /// <summary>
    /// The emitter instances in this effect.
    /// </summary>
    public List<EmitterInstance> Emitters { get; } = [];

    /// <summary>
    /// Number of particles to preload/warm up.
    /// </summary>
    public int PreloadCount { get; set; } = 0;

    /// <summary>
    /// Creates a new effect with a single emitter reference.
    /// </summary>
    public static EditableEffect CreateDefault(string name, string emitterName)
    {
        var effect = new EditableEffect { Name = name };
        effect.Emitters.Add(EmitterInstance.Create(emitterName));
        return effect;
    }

    /// <summary>
    /// Adds an emitter instance to this effect.
    /// </summary>
    public EmitterInstance AddEmitter(string emitterName)
    {
        var instance = EmitterInstance.Create(emitterName);
        Emitters.Add(instance);
        return instance;
    }

    /// <summary>
    /// Removes an emitter instance from this effect.
    /// </summary>
    public bool RemoveEmitter(EmitterInstance instance)
    {
        return Emitters.Remove(instance);
    }
}
