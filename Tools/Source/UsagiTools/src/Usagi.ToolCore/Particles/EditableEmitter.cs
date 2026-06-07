// Usagi.ToolCore - Particle Editor Support
// Editable emitter model

namespace Usagi.ToolCore.Particles;

/// <summary>
/// Blend function values matching engine enums.
/// </summary>
public enum BlendFunc
{
    Zero = 0,
    One = 1,
    SrcColor = 2,
    OneMinusSrcColor = 3,
    DstColor = 4,
    OneMinusDstColor = 5,
    SrcAlpha = 6,
    OneMinusSrcAlpha = 7,
    DstAlpha = 8,
    OneMinusDstAlpha = 9,
    ConstColor = 10,
    OneMinusConstColor = 11,
    ConstAlpha = 12,
    OneMinusConstAlpha = 13,
    SrcAlphaSaturate = 14
}

/// <summary>
/// Blend operation values.
/// </summary>
public enum BlendOp
{
    Add = 0,
    Subtract = 1,
    ReverseSubtract = 2,
    Min = 3,
    Max = 4
}

/// <summary>
/// Particle type values.
/// </summary>
public enum ParticleType
{
    Billboard = 0,
    DirectionOriented = 1,
    YAxisAligned = 2,
    VelocityAligned = 3,
    Trail = 4
}

/// <summary>
/// Emission type values.
/// </summary>
public enum EmissionType
{
    Continuous = 0,
    OneShot = 1,
    Infinite = 2
}

/// <summary>
/// Texture animation mode.
/// </summary>
public enum TextureAnimMode
{
    None = 0,
    Linear = 1,
    Random = 2
}

/// <summary>
/// Color mode for particle coloring.
/// </summary>
public enum ColorMode
{
    Constant = 0,
    Random = 1,
    Gradient = 2
}

/// <summary>
/// Emitter shape type.
/// </summary>
public enum EmitterShape
{
    Point = 0,
    Sphere = 1,
    Box = 2,
    Cylinder = 3,
    Cone = 4
}

/// <summary>
/// Blend state settings.
/// </summary>
public sealed class BlendSettings
{
    public BlendFunc RgbSrcFunc { get; set; } = BlendFunc.SrcAlpha;
    public BlendFunc RgbDestFunc { get; set; } = BlendFunc.OneMinusSrcAlpha;
    public BlendOp RgbOp { get; set; } = BlendOp.Add;
    public BlendFunc AlphaSrcFunc { get; set; } = BlendFunc.One;
    public BlendFunc AlphaDestFunc { get; set; } = BlendFunc.OneMinusSrcAlpha;
    public BlendOp AlphaOp { get; set; } = BlendOp.Add;
    public Color4 ConstColor { get; set; } = Color4.Clear;
    public int AlphaTestFunc { get; set; } = 1;
    public float AlphaTestReference { get; set; } = 0.0f;
}

/// <summary>
/// Sort/render settings.
/// </summary>
public sealed class SortSettings
{
    public int RenderLayer { get; set; } = 1;
    public int Priority { get; set; } = 0;
    public bool WriteDepth { get; set; } = false;
}

/// <summary>
/// Texture animation settings.
/// </summary>
public sealed class TextureAnimation
{
    public TextureAnimMode Mode { get; set; } = TextureAnimMode.None;
    public bool RandomOffset { get; set; } = false;
    public List<int> AnimIndex { get; } = [];
    public float AnimTimeScale { get; set; } = 1.0f;
}

/// <summary>
/// A texture slot in the emitter.
/// </summary>
public sealed class TextureSlot
{
    public string Name { get; set; } = "";
    public int PatternRepeatHor { get; set; } = 1;
    public int PatternRepeatVer { get; set; } = 1;
    public TextureAnimation Animation { get; } = new();
}

/// <summary>
/// Emission rate and timing settings.
/// </summary>
public sealed class EmissionSettings
{
    public EmissionType Type { get; set; } = EmissionType.Continuous;
    public float EmissionTime { get; set; } = 5.0f;
    public AnimatedFloat EmissionRate { get; } = new(10.0f);
    public float ReleaseInterval { get; set; } = 0.0f;
    public float ReleaseIntervalRandom { get; set; } = 0.0f;
    public int MaxParticles { get; set; } = 128;
    public Vec3 UserRotation { get; set; } = Vec3.Zero;
}

/// <summary>
/// Particle color settings.
/// </summary>
public sealed class ParticleColorSettings
{
    public ColorMode Mode { get; set; } = ColorMode.Constant;
    public Color4 Color0 { get; set; } = Color4.White;
    public Color4 Color1 { get; set; } = Color4.White;
    public Color4 Color2 { get; set; } = Color4.White;
    public float InTimeEnd { get; set; } = 0.0f;
    public float OutTimeStart { get; set; } = 1.0f;
    public float Peak { get; set; } = 1.0f;
    public int RepetitionCount { get; set; } = 0;
    public bool RandomRepetitionPos { get; set; } = false;
    public float LerpEnvColor { get; set; } = 0.0f;
}

/// <summary>
/// Particle alpha/transparency settings.
/// </summary>
public sealed class ParticleAlphaSettings
{
    public float InitialAlpha { get; set; } = 0.0f;
    public float IntermediateAlpha { get; set; } = 1.0f;
    public float EndAlpha { get; set; } = 0.0f;
    public float FinishInTime { get; set; } = 0.1f;
    public float OutStartTiming { get; set; } = 0.9f;
}

/// <summary>
/// Particle rotation settings.
/// </summary>
public sealed class ParticleRotationSettings
{
    public float BaseRotation { get; set; } = 0.0f;
    public float Randomise { get; set; } = 0.0f;
    public float Speed { get; set; } = 0.0f;
    public float SpeedRandomise { get; set; } = 0.0f;
}

/// <summary>
/// Particle scale settings.
/// </summary>
public sealed class ParticleScaleSettings
{
    public AnimatedFloat StandardValue { get; } = new(1.0f);
    public float Randomness { get; set; } = 0.0f;
    public float Initial { get; set; } = 1.0f;
    public float Intermediate { get; set; } = 1.0f;
    public float Ending { get; set; } = 1.0f;
    public float BeginScaleIn { get; set; } = 0.0f;
    public float StartScaleOut { get; set; } = 1.0f;
}

/// <summary>
/// Base shape transform settings.
/// </summary>
public sealed class BaseShapeSettings
{
    public Vec3 Scale { get; set; } = Vec3.One;
    public Vec3 Rotation { get; set; } = Vec3.Zero;
    public Vec3 Position { get; set; } = Vec3.Zero;
    public float Hollowness { get; set; } = 0.0f;
    public Vec3 Velocity { get; set; } = Vec3.Zero;
    public float SpeedRand { get; set; } = 0.0f;
    public Vec3 Gravity { get; set; } = Vec3.Zero;
    public float ShapeExpandVel { get; set; } = 0.0f;
}

/// <summary>
/// Arc settings for circular shapes.
/// </summary>
public sealed class ArcSettings
{
    public float ArcWidthDeg { get; set; } = 360.0f;
    public float ArcStartDeg { get; set; } = 0.0f;
    public bool RandomizeStartAngle { get; set; } = false;
}

/// <summary>
/// Shape details for an emitter.
/// </summary>
public sealed class EmitterShapeDetails
{
    public BaseShapeSettings BaseShape { get; } = new();
    public ArcSettings Arc { get; } = new();
    public Vec3 ShapeExtents { get; set; } = Vec3.Zero;
}

/// <summary>
/// A complete editable particle emitter.
/// </summary>
public sealed class EditableEmitter
{
    /// <summary>
    /// The emitter name (derived from filename).
    /// </summary>
    public string Name { get; set; } = "NewEmitter";

    /// <summary>
    /// Path to the source YAML file.
    /// </summary>
    public string SourcePath { get; set; } = "";

    // Rendering
    public BlendSettings Blend { get; } = new();
    public float SoftFadeDistance { get; set; } = 0.0f;
    public SortSettings Sort { get; } = new();
    public List<TextureSlot> Textures { get; } = [];

    // Emission
    public EmissionSettings Emission { get; } = new();
    public float PositionRandomness { get; set; } = 0.0f;

    // Velocity
    public AnimatedFloat OmniVelocity { get; } = new(0.0f);
    public AnimatedFloat DirVelocity { get; } = new(0.0f);
    public Vec3 VelocityDir { get; set; } = Vec3.Up;
    public float VelocityDirConeDeg { get; set; } = 0.0f;
    public float SpeedRandomness { get; set; } = 0.0f;
    public bool InheritVelocity { get; set; } = false;

    // Physics
    public bool LocalEffect { get; set; } = true;
    public bool CpuPositionUpdate { get; set; } = false;
    public float Drag { get; set; } = 0.0f;
    public float GravityStrength { get; set; } = 0.0f;
    public Vec3 GravityDir { get; set; } = Vec3.Down;

    // Particle settings
    public ParticleType Type { get; set; } = ParticleType.Billboard;
    public AnimatedFloat Life { get; } = new(1.0f);
    public float LifeRandomness { get; set; } = 0.0f;
    public Vec2 ParticleCenter { get; set; } = Vec2.Half;

    // Appearance
    public ParticleColorSettings Color { get; } = new();
    public ParticleAlphaSettings Alpha { get; } = new();
    public ParticleRotationSettings Rotation { get; } = new();
    public ParticleScaleSettings Scale { get; } = new();

    // Shape
    public EmitterShape Shape { get; set; } = EmitterShape.Point;
    public EmitterShapeDetails ShapeDetails { get; } = new();

    /// <summary>
    /// Creates a new emitter with default settings.
    /// </summary>
    public static EditableEmitter CreateDefault(string name = "NewEmitter")
    {
        var emitter = new EditableEmitter { Name = name };
        emitter.Textures.Add(new TextureSlot { Name = "particles/default" });
        return emitter;
    }
}
