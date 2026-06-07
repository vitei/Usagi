// Usagi.ToolCore - Particle Editor Support
// YAML parser for emitter and effect files

using System.Globalization;
using YamlDotNet.RepresentationModel;

namespace Usagi.ToolCore.Particles;

/// <summary>
/// Parses particle emitter and effect YAML files.
/// </summary>
public static class ParticleYamlParser
{
    /// <summary>
    /// Parses an emitter from a YAML file.
    /// </summary>
    public static EditableEmitter ParseEmitter(string path)
    {
        var content = File.ReadAllText(path);
        return ParseEmitterFromYaml(content, path);
    }

    /// <summary>
    /// Parses an emitter from YAML content.
    /// </summary>
    public static EditableEmitter ParseEmitterFromYaml(string yaml, string sourcePath = "")
    {
        var emitter = new EditableEmitter
        {
            SourcePath = sourcePath,
            Name = Path.GetFileNameWithoutExtension(sourcePath)
        };

        var input = new StringReader(yaml);
        var yamlStream = new YamlStream();
        yamlStream.Load(input);

        // Emitter files have two documents: EmitterEmission and EmitterShapeDetails
        foreach (var doc in yamlStream.Documents)
        {
            if (doc.RootNode is YamlMappingNode root)
            {
                foreach (var entry in root.Children)
                {
                    var key = GetScalarValue(entry.Key);
                    if (key == "Particles.EmitterEmission" && entry.Value is YamlMappingNode emission)
                    {
                        ParseEmitterEmission(emission, emitter);
                    }
                    else if (key == "Particles.EmitterShapeDetails" && entry.Value is YamlMappingNode shape)
                    {
                        ParseEmitterShapeDetails(shape, emitter.ShapeDetails);
                    }
                }
            }
        }

        return emitter;
    }

    /// <summary>
    /// Parses an effect from a YAML file.
    /// </summary>
    public static EditableEffect ParseEffect(string path)
    {
        var content = File.ReadAllText(path);
        return ParseEffectFromYaml(content, path);
    }

    /// <summary>
    /// Parses an effect from YAML content.
    /// </summary>
    public static EditableEffect ParseEffectFromYaml(string yaml, string sourcePath = "")
    {
        var effect = new EditableEffect
        {
            SourcePath = sourcePath,
            Name = Path.GetFileNameWithoutExtension(sourcePath)
        };

        var input = new StringReader(yaml);
        var yamlStream = new YamlStream();
        yamlStream.Load(input);

        if (yamlStream.Documents.Count > 0 && yamlStream.Documents[0].RootNode is YamlMappingNode root)
        {
            foreach (var entry in root.Children)
            {
                var key = GetScalarValue(entry.Key);
                if (key == "Particles.EffectGroup" && entry.Value is YamlMappingNode group)
                {
                    ParseEffectGroup(group, effect);
                }
            }
        }

        return effect;
    }

    private static void ParseEmitterEmission(YamlMappingNode node, EditableEmitter emitter)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "blend" when entry.Value is YamlMappingNode blend:
                    ParseBlendSettings(blend, emitter.Blend);
                    break;
                case "fSoftFadeDistance":
                    emitter.SoftFadeDistance = GetFloat(entry.Value);
                    break;
                case "sortSettings" when entry.Value is YamlMappingNode sort:
                    ParseSortSettings(sort, emitter.Sort);
                    break;
                case "textureData" when entry.Value is YamlSequenceNode textures:
                    ParseTextures(textures, emitter.Textures);
                    break;
                case "emission" when entry.Value is YamlMappingNode emission:
                    ParseEmissionSettings(emission, emitter.Emission);
                    break;
                case "fPositionRandomness":
                    emitter.PositionRandomness = GetFloat(entry.Value);
                    break;
                case "omniVelocity" when entry.Value is YamlMappingNode omni:
                    ParseAnimatedFloat(omni, emitter.OmniVelocity);
                    break;
                case "dirVelocity" when entry.Value is YamlMappingNode dir:
                    ParseAnimatedFloat(dir, emitter.DirVelocity);
                    break;
                case "vVelocityDir" when entry.Value is YamlMappingNode vec:
                    emitter.VelocityDir = ParseVec3(vec);
                    break;
                case "fVelocityDirConeDeg":
                    emitter.VelocityDirConeDeg = GetFloat(entry.Value);
                    break;
                case "fSpeedRandomness":
                    emitter.SpeedRandomness = GetFloat(entry.Value);
                    break;
                case "bInheritVelocity":
                    emitter.InheritVelocity = GetBool(entry.Value);
                    break;
                case "bLocalEffect":
                    emitter.LocalEffect = GetBool(entry.Value);
                    break;
                case "bCPUPositionUpdate":
                    emitter.CpuPositionUpdate = GetBool(entry.Value);
                    break;
                case "fDrag":
                    emitter.Drag = GetFloat(entry.Value);
                    break;
                case "fGravityStrength":
                    emitter.GravityStrength = GetFloat(entry.Value);
                    break;
                case "vGravityDir" when entry.Value is YamlMappingNode grav:
                    emitter.GravityDir = ParseVec3(grav);
                    break;
                case "eParticleType":
                    emitter.Type = (ParticleType)GetInt(entry.Value);
                    break;
                case "life" when entry.Value is YamlMappingNode life:
                    ParseAnimatedFloat(life, emitter.Life);
                    break;
                case "fLifeRandomness":
                    emitter.LifeRandomness = GetFloat(entry.Value);
                    break;
                case "vParticleCenter" when entry.Value is YamlMappingNode center:
                    emitter.ParticleCenter = ParseVec2(center);
                    break;
                case "particleColor" when entry.Value is YamlMappingNode color:
                    ParseParticleColor(color, emitter.Color);
                    break;
                case "particleAlpha" when entry.Value is YamlMappingNode alpha:
                    ParseParticleAlpha(alpha, emitter.Alpha);
                    break;
                case "particleRotation" when entry.Value is YamlMappingNode rot:
                    ParseParticleRotation(rot, emitter.Rotation);
                    break;
                case "particleScale" when entry.Value is YamlMappingNode scale:
                    ParseParticleScale(scale, emitter.Scale);
                    break;
                case "eShape":
                    emitter.Shape = (EmitterShape)GetInt(entry.Value);
                    break;
            }
        }
    }

    private static void ParseBlendSettings(YamlMappingNode node, BlendSettings blend)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "rgbSrcFunc":
                    blend.RgbSrcFunc = (BlendFunc)GetInt(entry.Value);
                    break;
                case "rgbDestFunc":
                    blend.RgbDestFunc = (BlendFunc)GetInt(entry.Value);
                    break;
                case "rgbOp":
                    blend.RgbOp = (BlendOp)GetInt(entry.Value);
                    break;
                case "alphaSrcFunc":
                    blend.AlphaSrcFunc = (BlendFunc)GetInt(entry.Value);
                    break;
                case "alphaDestFunc":
                    blend.AlphaDestFunc = (BlendFunc)GetInt(entry.Value);
                    break;
                case "alphaOp":
                    blend.AlphaOp = (BlendOp)GetInt(entry.Value);
                    break;
                case "constColor" when entry.Value is YamlMappingNode color:
                    blend.ConstColor = ParseColor4(color);
                    break;
                case "alphaTestFunc":
                    blend.AlphaTestFunc = GetInt(entry.Value);
                    break;
                case "alphaTestReference":
                    blend.AlphaTestReference = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseSortSettings(YamlMappingNode node, SortSettings sort)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "eRenderLayer":
                    sort.RenderLayer = GetInt(entry.Value);
                    break;
                case "uPriority":
                    sort.Priority = GetInt(entry.Value);
                    break;
                case "bWriteDepth":
                    sort.WriteDepth = GetBool(entry.Value);
                    break;
            }
        }
    }

    private static void ParseTextures(YamlSequenceNode node, List<TextureSlot> textures)
    {
        foreach (var item in node.Children)
        {
            if (item is YamlMappingNode tex)
            {
                var slot = new TextureSlot();
                foreach (var entry in tex.Children)
                {
                    var key = GetScalarValue(entry.Key);
                    switch (key)
                    {
                        case "name":
                            slot.Name = GetScalarValue(entry.Value);
                            break;
                        case "uPatternRepeatHor":
                            slot.PatternRepeatHor = GetInt(entry.Value);
                            break;
                        case "uPatternRepeatVer":
                            slot.PatternRepeatVer = GetInt(entry.Value);
                            break;
                        case "textureAnim" when entry.Value is YamlMappingNode anim:
                            ParseTextureAnimation(anim, slot.Animation);
                            break;
                    }
                }
                textures.Add(slot);
            }
        }
    }

    private static void ParseTextureAnimation(YamlMappingNode node, TextureAnimation anim)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "eTexMode":
                    anim.Mode = (TextureAnimMode)GetInt(entry.Value);
                    break;
                case "bRandomOffset":
                    anim.RandomOffset = GetBool(entry.Value);
                    break;
                case "animIndex" when entry.Value is YamlSequenceNode indices:
                    anim.AnimIndex.Clear();
                    foreach (var idx in indices.Children)
                    {
                        anim.AnimIndex.Add(GetInt(idx));
                    }
                    break;
                case "fAnimTimeScale":
                    anim.AnimTimeScale = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseEmissionSettings(YamlMappingNode node, EmissionSettings emission)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "eEmissionType":
                    emission.Type = (EmissionType)GetInt(entry.Value);
                    break;
                case "fEmissionTime":
                    emission.EmissionTime = GetFloat(entry.Value);
                    break;
                case "emissionRate" when entry.Value is YamlMappingNode rate:
                    ParseAnimatedFloat(rate, emission.EmissionRate);
                    break;
                case "fReleaseInterval":
                    emission.ReleaseInterval = GetFloat(entry.Value);
                    break;
                case "fReleaseIntervalRandom":
                    emission.ReleaseIntervalRandom = GetFloat(entry.Value);
                    break;
                case "uMaxParticles":
                    emission.MaxParticles = GetInt(entry.Value);
                    break;
                case "vUserRotation" when entry.Value is YamlMappingNode rot:
                    emission.UserRotation = ParseVec3(rot);
                    break;
            }
        }
    }

    private static void ParseAnimatedFloat(YamlMappingNode node, AnimatedFloat anim)
    {
        anim.Frames.Clear();
        if (node.Children.TryGetValue(new YamlScalarNode("frames"), out var framesNode)
            && framesNode is YamlSequenceNode frames)
        {
            foreach (var frame in frames.Children)
            {
                if (frame is YamlMappingNode frameNode)
                {
                    var keyframe = new Keyframe();
                    foreach (var entry in frameNode.Children)
                    {
                        var key = GetScalarValue(entry.Key);
                        switch (key)
                        {
                            case "fTimeIndex":
                                keyframe.TimeIndex = GetFloat(entry.Value);
                                break;
                            case "fValue":
                                keyframe.Value = GetFloat(entry.Value);
                                break;
                        }
                    }
                    anim.Frames.Add(keyframe);
                }
            }
        }
    }

    private static void ParseParticleColor(YamlMappingNode node, ParticleColorSettings color)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "eColorMode":
                    color.Mode = (ColorMode)GetInt(entry.Value);
                    break;
                case "cColor0" when entry.Value is YamlMappingNode c:
                    color.Color0 = ParseColor4(c);
                    break;
                case "cColor1" when entry.Value is YamlMappingNode c:
                    color.Color1 = ParseColor4(c);
                    break;
                case "cColor2" when entry.Value is YamlMappingNode c:
                    color.Color2 = ParseColor4(c);
                    break;
                case "fInTimeEnd":
                    color.InTimeEnd = GetFloat(entry.Value);
                    break;
                case "fOutTimeStart":
                    color.OutTimeStart = GetFloat(entry.Value);
                    break;
                case "fPeak":
                    color.Peak = GetFloat(entry.Value);
                    break;
                case "uRepetitionCount":
                    color.RepetitionCount = GetInt(entry.Value);
                    break;
                case "bRandomRepetitionPos":
                    color.RandomRepetitionPos = GetBool(entry.Value);
                    break;
                case "fLerpEnvColor":
                    color.LerpEnvColor = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseParticleAlpha(YamlMappingNode node, ParticleAlphaSettings alpha)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "fInitialAlpha":
                    alpha.InitialAlpha = GetFloat(entry.Value);
                    break;
                case "fIntermediateAlpha":
                    alpha.IntermediateAlpha = GetFloat(entry.Value);
                    break;
                case "fEndAlpha":
                    alpha.EndAlpha = GetFloat(entry.Value);
                    break;
                case "fFinishInTime":
                    alpha.FinishInTime = GetFloat(entry.Value);
                    break;
                case "fOutStartTiming":
                    alpha.OutStartTiming = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseParticleRotation(YamlMappingNode node, ParticleRotationSettings rot)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "fBaseRotation":
                    rot.BaseRotation = GetFloat(entry.Value);
                    break;
                case "fRandomise":
                    rot.Randomise = GetFloat(entry.Value);
                    break;
                case "fSpeed":
                    rot.Speed = GetFloat(entry.Value);
                    break;
                case "fSpeedRandomise":
                    rot.SpeedRandomise = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseParticleScale(YamlMappingNode node, ParticleScaleSettings scale)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "standardValue" when entry.Value is YamlMappingNode std:
                    ParseAnimatedFloat(std, scale.StandardValue);
                    break;
                case "fRandomness":
                    scale.Randomness = GetFloat(entry.Value);
                    break;
                case "fInitial":
                    scale.Initial = GetFloat(entry.Value);
                    break;
                case "fIntermediate":
                    scale.Intermediate = GetFloat(entry.Value);
                    break;
                case "fEnding":
                    scale.Ending = GetFloat(entry.Value);
                    break;
                case "fBeginScaleIn":
                    scale.BeginScaleIn = GetFloat(entry.Value);
                    break;
                case "fStartScaleOut":
                    scale.StartScaleOut = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseEmitterShapeDetails(YamlMappingNode node, EmitterShapeDetails details)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "baseShape" when entry.Value is YamlMappingNode shape:
                    ParseBaseShape(shape, details.BaseShape);
                    break;
                case "arc" when entry.Value is YamlMappingNode arc:
                    ParseArc(arc, details.Arc);
                    break;
                case "vShapeExtents" when entry.Value is YamlMappingNode ext:
                    details.ShapeExtents = ParseVec3(ext);
                    break;
            }
        }
    }

    private static void ParseBaseShape(YamlMappingNode node, BaseShapeSettings shape)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "vScale" when entry.Value is YamlMappingNode v:
                    shape.Scale = ParseVec3(v);
                    break;
                case "vRotation" when entry.Value is YamlMappingNode v:
                    shape.Rotation = ParseVec3(v);
                    break;
                case "vPosition" when entry.Value is YamlMappingNode v:
                    shape.Position = ParseVec3(v);
                    break;
                case "fHollowness":
                    shape.Hollowness = GetFloat(entry.Value);
                    break;
                case "vVelocity" when entry.Value is YamlMappingNode v:
                    shape.Velocity = ParseVec3(v);
                    break;
                case "fSpeedRand":
                    shape.SpeedRand = GetFloat(entry.Value);
                    break;
                case "vGravity" when entry.Value is YamlMappingNode v:
                    shape.Gravity = ParseVec3(v);
                    break;
                case "fShapeExpandVel":
                    shape.ShapeExpandVel = GetFloat(entry.Value);
                    break;
            }
        }
    }

    private static void ParseArc(YamlMappingNode node, ArcSettings arc)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "fArcWidthDeg":
                    arc.ArcWidthDeg = GetFloat(entry.Value);
                    break;
                case "fArcStartDeg":
                    arc.ArcStartDeg = GetFloat(entry.Value);
                    break;
                case "bRandomizeStartAngle":
                    arc.RandomizeStartAngle = GetBool(entry.Value);
                    break;
            }
        }
    }

    private static void ParseEffectGroup(YamlMappingNode node, EditableEffect effect)
    {
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "emitters" when entry.Value is YamlSequenceNode emitters:
                    foreach (var item in emitters.Children)
                    {
                        if (item is YamlMappingNode em)
                        {
                            var instance = new EmitterInstance();
                            foreach (var e in em.Children)
                            {
                                var k = GetScalarValue(e.Key);
                                switch (k)
                                {
                                    case "emitterName":
                                        instance.EmitterName = GetScalarValue(e.Value);
                                        break;
                                    case "vScale" when e.Value is YamlMappingNode v:
                                        instance.Scale = ParseVec3(v);
                                        break;
                                    case "vRotation" when e.Value is YamlMappingNode v:
                                        instance.Rotation = ParseVec3(v);
                                        break;
                                    case "vPosition" when e.Value is YamlMappingNode v:
                                        instance.Position = ParseVec3(v);
                                        break;
                                    case "fParticleScale":
                                        instance.ParticleScale = GetFloat(e.Value);
                                        break;
                                    case "fReleaseFrame":
                                        instance.ReleaseFrame = GetFloat(e.Value);
                                        break;
                                }
                            }
                            effect.Emitters.Add(instance);
                        }
                    }
                    break;
                case "uPreloadCount":
                    effect.PreloadCount = GetInt(entry.Value);
                    break;
            }
        }
    }

    // Helper methods
    private static string GetScalarValue(YamlNode node) =>
        node is YamlScalarNode scalar ? scalar.Value ?? "" : "";

    private static int GetInt(YamlNode node) =>
        int.TryParse(GetScalarValue(node), NumberStyles.Integer, CultureInfo.InvariantCulture, out var val) ? val : 0;

    private static float GetFloat(YamlNode node) =>
        float.TryParse(GetScalarValue(node), NumberStyles.Float, CultureInfo.InvariantCulture, out var val) ? val : 0f;

    private static bool GetBool(YamlNode node) =>
        GetScalarValue(node).Equals("true", StringComparison.OrdinalIgnoreCase);

    private static Vec3 ParseVec3(YamlMappingNode node)
    {
        var vec = Vec3.Zero;
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "x": vec.X = GetFloat(entry.Value); break;
                case "y": vec.Y = GetFloat(entry.Value); break;
                case "z": vec.Z = GetFloat(entry.Value); break;
            }
        }
        return vec;
    }

    private static Vec2 ParseVec2(YamlMappingNode node)
    {
        var vec = Vec2.Zero;
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "x": vec.X = GetFloat(entry.Value); break;
                case "y": vec.Y = GetFloat(entry.Value); break;
            }
        }
        return vec;
    }

    private static Color4 ParseColor4(YamlMappingNode node)
    {
        var color = Color4.Clear;
        foreach (var entry in node.Children)
        {
            var key = GetScalarValue(entry.Key);
            switch (key)
            {
                case "m_fR": color.R = GetFloat(entry.Value); break;
                case "m_fG": color.G = GetFloat(entry.Value); break;
                case "m_fB": color.B = GetFloat(entry.Value); break;
                case "m_fA": color.A = GetFloat(entry.Value); break;
            }
        }
        return color;
    }
}
