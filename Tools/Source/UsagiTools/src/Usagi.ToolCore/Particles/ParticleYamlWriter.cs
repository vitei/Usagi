// Usagi.ToolCore - Particle Editor Support
// YAML writer for emitter and effect files

using System.Globalization;
using System.Text;

namespace Usagi.ToolCore.Particles;

/// <summary>
/// Writes particle emitter and effect data to YAML.
/// </summary>
public static class ParticleYamlWriter
{
    /// <summary>
    /// Saves an emitter to a YAML file.
    /// </summary>
    public static void SaveEmitter(EditableEmitter emitter, string path)
    {
        var yaml = WriteEmitter(emitter);
        File.WriteAllText(path, yaml);
    }

    /// <summary>
    /// Writes an emitter to YAML string.
    /// </summary>
    public static string WriteEmitter(EditableEmitter emitter)
    {
        var sb = new StringBuilder();

        // Write EmitterEmission document
        sb.AppendLine("Particles.EmitterEmission:");
        WriteEmitterEmission(sb, emitter, "  ");

        // Document separator
        sb.AppendLine("---");

        // Write EmitterShapeDetails document
        sb.AppendLine("Particles.EmitterShapeDetails:");
        WriteEmitterShapeDetails(sb, emitter.ShapeDetails, "  ");

        return sb.ToString();
    }

    /// <summary>
    /// Saves an effect to a YAML file.
    /// </summary>
    public static void SaveEffect(EditableEffect effect, string path)
    {
        var yaml = WriteEffect(effect);
        File.WriteAllText(path, yaml);
    }

    /// <summary>
    /// Writes an effect to YAML string.
    /// </summary>
    public static string WriteEffect(EditableEffect effect)
    {
        var sb = new StringBuilder();

        sb.AppendLine("Particles.EffectGroup:");
        sb.AppendLine("  emitters:");

        foreach (var em in effect.Emitters)
        {
            sb.AppendLine($"    - emitterName: {em.EmitterName}");
            WriteVec3(sb, "vScale", em.Scale, "      ");
            WriteVec3(sb, "vRotation", em.Rotation, "      ");
            WriteVec3(sb, "vPosition", em.Position, "      ");
            sb.AppendLine($"      fParticleScale: {F(em.ParticleScale)}");
            sb.AppendLine($"      fReleaseFrame: {F(em.ReleaseFrame)}");
        }

        sb.AppendLine($"  uPreloadCount: {effect.PreloadCount}");

        return sb.ToString();
    }

    private static void WriteEmitterEmission(StringBuilder sb, EditableEmitter emitter, string indent)
    {
        // Blend settings
        sb.AppendLine($"{indent}blend:");
        WriteBlendSettings(sb, emitter.Blend, indent + "  ");

        sb.AppendLine($"{indent}fSoftFadeDistance: {F(emitter.SoftFadeDistance)}");

        // Sort settings
        sb.AppendLine($"{indent}sortSettings:");
        WriteSortSettings(sb, emitter.Sort, indent + "  ");

        // Textures
        sb.AppendLine($"{indent}textureData:");
        foreach (var tex in emitter.Textures)
        {
            sb.AppendLine($"{indent}  - name: {tex.Name}");
            sb.AppendLine($"{indent}    uPatternRepeatHor: {tex.PatternRepeatHor}");
            sb.AppendLine($"{indent}    uPatternRepeatVer: {tex.PatternRepeatVer}");
            sb.AppendLine($"{indent}    textureAnim:");
            sb.AppendLine($"{indent}      eTexMode: {(int)tex.Animation.Mode}");
            sb.AppendLine($"{indent}      bRandomOffset: {B(tex.Animation.RandomOffset)}");
            if (tex.Animation.AnimIndex.Count > 0)
            {
                sb.AppendLine($"{indent}      animIndex: [{string.Join(", ", tex.Animation.AnimIndex)}]");
            }
            else
            {
                sb.AppendLine($"{indent}      animIndex: []");
            }
            sb.AppendLine($"{indent}      fAnimTimeScale: {F(tex.Animation.AnimTimeScale)}");
        }

        // Emission
        sb.AppendLine($"{indent}emission:");
        WriteEmissionSettings(sb, emitter.Emission, indent + "  ");

        sb.AppendLine($"{indent}fPositionRandomness: {F(emitter.PositionRandomness)}");

        // Velocity
        WriteAnimatedFloat(sb, "omniVelocity", emitter.OmniVelocity, indent);
        WriteAnimatedFloat(sb, "dirVelocity", emitter.DirVelocity, indent);
        WriteVec3(sb, "vVelocityDir", emitter.VelocityDir, indent);
        sb.AppendLine($"{indent}fVelocityDirConeDeg: {F(emitter.VelocityDirConeDeg)}");
        sb.AppendLine($"{indent}fSpeedRandomness: {F(emitter.SpeedRandomness)}");
        sb.AppendLine($"{indent}bInheritVelocity: {B(emitter.InheritVelocity)}");
        sb.AppendLine($"{indent}bLocalEffect: {B(emitter.LocalEffect)}");
        sb.AppendLine($"{indent}bCPUPositionUpdate: {B(emitter.CpuPositionUpdate)}");
        sb.AppendLine($"{indent}fDrag: {F(emitter.Drag)}");
        sb.AppendLine($"{indent}fGravityStrength: {F(emitter.GravityStrength)}");
        WriteVec3(sb, "vGravityDir", emitter.GravityDir, indent);

        // Particle settings
        sb.AppendLine($"{indent}eParticleType: {(int)emitter.Type}");
        WriteAnimatedFloat(sb, "life", emitter.Life, indent);
        sb.AppendLine($"{indent}fLifeRandomness: {F(emitter.LifeRandomness)}");
        WriteVec2(sb, "vParticleCenter", emitter.ParticleCenter, indent);

        // Color
        sb.AppendLine($"{indent}particleColor:");
        WriteParticleColor(sb, emitter.Color, indent + "  ");

        // Alpha
        sb.AppendLine($"{indent}particleAlpha:");
        WriteParticleAlpha(sb, emitter.Alpha, indent + "  ");

        // Rotation
        sb.AppendLine($"{indent}particleRotation:");
        WriteParticleRotation(sb, emitter.Rotation, indent + "  ");

        // Scale
        sb.AppendLine($"{indent}particleScale:");
        WriteParticleScale(sb, emitter.Scale, indent + "  ");

        // Shape
        sb.AppendLine($"{indent}eShape: {(int)emitter.Shape}");
    }

    private static void WriteBlendSettings(StringBuilder sb, BlendSettings blend, string indent)
    {
        sb.AppendLine($"{indent}rgbSrcFunc: {(int)blend.RgbSrcFunc}");
        sb.AppendLine($"{indent}rgbDestFunc: {(int)blend.RgbDestFunc}");
        sb.AppendLine($"{indent}rgbOp: {(int)blend.RgbOp}");
        sb.AppendLine($"{indent}alphaSrcFunc: {(int)blend.AlphaSrcFunc}");
        sb.AppendLine($"{indent}alphaDestFunc: {(int)blend.AlphaDestFunc}");
        sb.AppendLine($"{indent}alphaOp: {(int)blend.AlphaOp}");
        WriteColor4(sb, "constColor", blend.ConstColor, indent);
        sb.AppendLine($"{indent}alphaTestFunc: {blend.AlphaTestFunc}");
        sb.AppendLine($"{indent}alphaTestReference: {F(blend.AlphaTestReference)}");
    }

    private static void WriteSortSettings(StringBuilder sb, SortSettings sort, string indent)
    {
        sb.AppendLine($"{indent}eRenderLayer: {sort.RenderLayer}");
        sb.AppendLine($"{indent}uPriority: {sort.Priority}");
        sb.AppendLine($"{indent}bWriteDepth: {B(sort.WriteDepth)}");
    }

    private static void WriteEmissionSettings(StringBuilder sb, EmissionSettings emission, string indent)
    {
        sb.AppendLine($"{indent}eEmissionType: {(int)emission.Type}");
        sb.AppendLine($"{indent}fEmissionTime: {F(emission.EmissionTime)}");
        WriteAnimatedFloat(sb, "emissionRate", emission.EmissionRate, indent);
        sb.AppendLine($"{indent}fReleaseInterval: {F(emission.ReleaseInterval)}");
        sb.AppendLine($"{indent}fReleaseIntervalRandom: {F(emission.ReleaseIntervalRandom)}");
        sb.AppendLine($"{indent}uMaxParticles: {emission.MaxParticles}");
        WriteVec3(sb, "vUserRotation", emission.UserRotation, indent);
    }

    private static void WriteParticleColor(StringBuilder sb, ParticleColorSettings color, string indent)
    {
        sb.AppendLine($"{indent}eColorMode: {(int)color.Mode}");
        WriteColor4(sb, "cColor0", color.Color0, indent);
        WriteColor4(sb, "cColor1", color.Color1, indent);
        WriteColor4(sb, "cColor2", color.Color2, indent);
        sb.AppendLine($"{indent}fInTimeEnd: {F(color.InTimeEnd)}");
        sb.AppendLine($"{indent}fOutTimeStart: {F(color.OutTimeStart)}");
        sb.AppendLine($"{indent}fPeak: {F(color.Peak)}");
        sb.AppendLine($"{indent}uRepetitionCount: {color.RepetitionCount}");
        sb.AppendLine($"{indent}bRandomRepetitionPos: {B(color.RandomRepetitionPos)}");
        sb.AppendLine($"{indent}fLerpEnvColor: {F(color.LerpEnvColor)}");
    }

    private static void WriteParticleAlpha(StringBuilder sb, ParticleAlphaSettings alpha, string indent)
    {
        sb.AppendLine($"{indent}fInitialAlpha: {F(alpha.InitialAlpha)}");
        sb.AppendLine($"{indent}fIntermediateAlpha: {F(alpha.IntermediateAlpha)}");
        sb.AppendLine($"{indent}fEndAlpha: {F(alpha.EndAlpha)}");
        sb.AppendLine($"{indent}fFinishInTime: {F(alpha.FinishInTime)}");
        sb.AppendLine($"{indent}fOutStartTiming: {F(alpha.OutStartTiming)}");
    }

    private static void WriteParticleRotation(StringBuilder sb, ParticleRotationSettings rot, string indent)
    {
        sb.AppendLine($"{indent}fBaseRotation: {F(rot.BaseRotation)}");
        sb.AppendLine($"{indent}fRandomise: {F(rot.Randomise)}");
        sb.AppendLine($"{indent}fSpeed: {F(rot.Speed)}");
        sb.AppendLine($"{indent}fSpeedRandomise: {F(rot.SpeedRandomise)}");
    }

    private static void WriteParticleScale(StringBuilder sb, ParticleScaleSettings scale, string indent)
    {
        WriteAnimatedFloat(sb, "standardValue", scale.StandardValue, indent);
        sb.AppendLine($"{indent}fRandomness: {F(scale.Randomness)}");
        sb.AppendLine($"{indent}fInitial: {F(scale.Initial)}");
        sb.AppendLine($"{indent}fIntermediate: {F(scale.Intermediate)}");
        sb.AppendLine($"{indent}fEnding: {F(scale.Ending)}");
        sb.AppendLine($"{indent}fBeginScaleIn: {F(scale.BeginScaleIn)}");
        sb.AppendLine($"{indent}fStartScaleOut: {F(scale.StartScaleOut)}");
    }

    private static void WriteEmitterShapeDetails(StringBuilder sb, EmitterShapeDetails details, string indent)
    {
        sb.AppendLine($"{indent}baseShape:");
        var bi = indent + "  ";
        WriteVec3(sb, "vScale", details.BaseShape.Scale, bi);
        WriteVec3(sb, "vRotation", details.BaseShape.Rotation, bi);
        WriteVec3(sb, "vPosition", details.BaseShape.Position, bi);
        sb.AppendLine($"{bi}fHollowness: {F(details.BaseShape.Hollowness)}");
        WriteVec3(sb, "vVelocity", details.BaseShape.Velocity, bi);
        sb.AppendLine($"{bi}fSpeedRand: {F(details.BaseShape.SpeedRand)}");
        WriteVec3(sb, "vGravity", details.BaseShape.Gravity, bi);
        sb.AppendLine($"{bi}fShapeExpandVel: {F(details.BaseShape.ShapeExpandVel)}");

        sb.AppendLine($"{indent}arc:");
        var ai = indent + "  ";
        sb.AppendLine($"{ai}fArcWidthDeg: {F(details.Arc.ArcWidthDeg)}");
        sb.AppendLine($"{ai}fArcStartDeg: {F(details.Arc.ArcStartDeg)}");
        sb.AppendLine($"{ai}bRandomizeStartAngle: {B(details.Arc.RandomizeStartAngle)}");

        WriteVec3(sb, "vShapeExtents", details.ShapeExtents, indent);
    }

    private static void WriteAnimatedFloat(StringBuilder sb, string name, AnimatedFloat anim, string indent)
    {
        sb.AppendLine($"{indent}{name}:");
        sb.AppendLine($"{indent}  frames:");
        foreach (var frame in anim.Frames)
        {
            sb.AppendLine($"{indent}    - fTimeIndex: {F(frame.TimeIndex)}");
            sb.AppendLine($"{indent}      fValue: {F(frame.Value)}");
        }
    }

    private static void WriteVec3(StringBuilder sb, string name, Vec3 vec, string indent)
    {
        sb.AppendLine($"{indent}{name}:");
        sb.AppendLine($"{indent}  x: {F(vec.X)}");
        sb.AppendLine($"{indent}  y: {F(vec.Y)}");
        sb.AppendLine($"{indent}  z: {F(vec.Z)}");
    }

    private static void WriteVec2(StringBuilder sb, string name, Vec2 vec, string indent)
    {
        sb.AppendLine($"{indent}{name}:");
        sb.AppendLine($"{indent}  x: {F(vec.X)}");
        sb.AppendLine($"{indent}  y: {F(vec.Y)}");
    }

    private static void WriteColor4(StringBuilder sb, string name, Color4 color, string indent)
    {
        sb.AppendLine($"{indent}{name}:");
        sb.AppendLine($"{indent}  m_fR: {F(color.R)}");
        sb.AppendLine($"{indent}  m_fG: {F(color.G)}");
        sb.AppendLine($"{indent}  m_fB: {F(color.B)}");
        sb.AppendLine($"{indent}  m_fA: {F(color.A)}");
    }

    // Format float to avoid locale issues
    private static string F(float value) =>
        value.ToString("0.0###############", CultureInfo.InvariantCulture);

    // Format bool as lowercase
    private static string B(bool value) => value ? "true" : "false";
}
