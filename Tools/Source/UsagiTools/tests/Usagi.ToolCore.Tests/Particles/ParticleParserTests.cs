// Usagi.ToolCore.Tests - Particle Parser Tests

using System.Globalization;
using Usagi.ToolCore.Particles;
using Xunit;

namespace Usagi.ToolCore.Tests.Particles;

public class ParticleParserTests
{
    private const string SampleEmitterYaml = """
        Particles.EmitterEmission:
          blend:
            rgbSrcFunc: 6
            rgbDestFunc: 7
            rgbOp: 0
            alphaSrcFunc: 1
            alphaDestFunc: 7
            alphaOp: 0
            constColor:
              m_fR: 0.0
              m_fG: 0.0
              m_fB: 0.0
              m_fA: 0.0
            alphaTestFunc: 1
            alphaTestReference: 0.0
          fSoftFadeDistance: 0.5
          sortSettings:
            eRenderLayer: 1
            uPriority: 5
            bWriteDepth: false
          textureData:
            - name: particles/spark
              uPatternRepeatHor: 2
              uPatternRepeatVer: 2
              textureAnim:
                eTexMode: 2
                bRandomOffset: true
                animIndex: [0, 1, 2, 3]
                fAnimTimeScale: 0.2
          emission:
            eEmissionType: 1
            fEmissionTime: 3.0
            emissionRate:
              frames:
                - fTimeIndex: 0.0
                  fValue: 25.0
            fReleaseInterval: 0.0
            fReleaseIntervalRandom: 0.0
            uMaxParticles: 64
            vUserRotation:
              x: 0.0
              y: 0.0
              z: 0.0
          fPositionRandomness: 0.1
          omniVelocity:
            frames:
              - fTimeIndex: 0.0
                fValue: 0.5
          dirVelocity:
            frames:
              - fTimeIndex: 0.0
                fValue: 1.0
          vVelocityDir:
            x: 0.0
            y: 1.0
            z: 0.0
          fVelocityDirConeDeg: 30.0
          fSpeedRandomness: 0.2
          bInheritVelocity: false
          bLocalEffect: true
          bCPUPositionUpdate: false
          fDrag: 0.05
          fGravityStrength: 0.5
          vGravityDir:
            x: 0.0
            y: -1.0
            z: 0.0
          eParticleType: 0
          life:
            frames:
              - fTimeIndex: 0.0
                fValue: 2.0
          fLifeRandomness: 0.3
          vParticleCenter:
            x: 0.5
            y: 0.5
          particleColor:
            eColorMode: 0
            cColor0:
              m_fR: 1.0
              m_fG: 0.8
              m_fB: 0.2
              m_fA: 1.0
            cColor1:
              m_fR: 1.0
              m_fG: 0.5
              m_fB: 0.1
              m_fA: 1.0
            cColor2:
              m_fR: 1.0
              m_fG: 1.0
              m_fB: 1.0
              m_fA: 1.0
            fInTimeEnd: 0.0
            fOutTimeStart: 1.0
            fPeak: 1.0
            uRepetitionCount: 0
            bRandomRepetitionPos: false
            fLerpEnvColor: 0.0
          particleAlpha:
            fInitialAlpha: 0.0
            fIntermediateAlpha: 1.0
            fEndAlpha: 0.0
            fFinishInTime: 0.1
            fOutStartTiming: 0.8
          particleRotation:
            fBaseRotation: 0.0
            fRandomise: 360.0
            fSpeed: 45.0
            fSpeedRandomise: 20.0
          particleScale:
            standardValue:
              frames:
                - fTimeIndex: 0.0
                  fValue: 0.5
            fRandomness: 0.2
            fInitial: 0.5
            fIntermediate: 1.0
            fEnding: 0.2
            fBeginScaleIn: 0.1
            fStartScaleOut: 0.7
          eShape: 1
        ---
        Particles.EmitterShapeDetails:
          baseShape:
            vScale:
              x: 1.0
              y: 1.0
              z: 1.0
            vRotation:
              x: 0.0
              y: 0.0
              z: 0.0
            vPosition:
              x: 0.0
              y: 0.0
              z: 0.0
            fHollowness: 0.5
            vVelocity:
              x: 0.0
              y: 0.0
              z: 0.0
            fSpeedRand: 0.0
            vGravity:
              x: 0.0
              y: 0.0
              z: 0.0
            fShapeExpandVel: 0.0
          arc:
            fArcWidthDeg: 360.0
            fArcStartDeg: 0.0
            bRandomizeStartAngle: false
          vShapeExtents:
            x: 0.0
            y: 0.0
            z: 0.0
        """;

    private const string SampleEffectYaml = """
        Particles.EffectGroup:
          emitters:
            - emitterName: spark_burst
              vScale:
                x: 1.0
                y: 1.0
                z: 1.0
              vRotation:
                x: 0.0
                y: 0.0
                z: 0.0
              vPosition:
                x: 0.0
                y: 0.5
                z: 0.0
              fParticleScale: 1.5
              fReleaseFrame: 0.0
            - emitterName: smoke_trail
              vScale:
                x: 2.0
                y: 2.0
                z: 2.0
              vRotation:
                x: 0.0
                y: 45.0
                z: 0.0
              vPosition:
                x: 0.0
                y: 0.0
                z: 0.0
              fParticleScale: 1.0
              fReleaseFrame: 0.5
          uPreloadCount: 2
        """;

    [Fact]
    public void ParseEmitter_ParsesBasicProperties()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(0.5f, emitter.SoftFadeDistance);
        Assert.Equal(EmitterShape.Sphere, emitter.Shape);
        Assert.Equal(ParticleType.Billboard, emitter.Type);
        Assert.True(emitter.LocalEffect);
    }

    [Fact]
    public void ParseEmitter_ParsesBlendSettings()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(BlendFunc.SrcAlpha, emitter.Blend.RgbSrcFunc);
        Assert.Equal(BlendFunc.OneMinusSrcAlpha, emitter.Blend.RgbDestFunc);
        Assert.Equal(BlendOp.Add, emitter.Blend.RgbOp);
    }

    [Fact]
    public void ParseEmitter_ParsesSortSettings()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(1, emitter.Sort.RenderLayer);
        Assert.Equal(5, emitter.Sort.Priority);
        Assert.False(emitter.Sort.WriteDepth);
    }

    [Fact]
    public void ParseEmitter_ParsesTextures()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Single(emitter.Textures);
        Assert.Equal("particles/spark", emitter.Textures[0].Name);
        Assert.Equal(2, emitter.Textures[0].PatternRepeatHor);
        Assert.Equal(TextureAnimMode.Random, emitter.Textures[0].Animation.Mode);
        Assert.True(emitter.Textures[0].Animation.RandomOffset);
        Assert.Equal([0, 1, 2, 3], emitter.Textures[0].Animation.AnimIndex);
    }

    [Fact]
    public void ParseEmitter_UsesInvariantFloatFormatting()
    {
        var originalCulture = CultureInfo.CurrentCulture;
        var originalUiCulture = CultureInfo.CurrentUICulture;

        try
        {
            CultureInfo.CurrentCulture = CultureInfo.GetCultureInfo("fr-FR");
            CultureInfo.CurrentUICulture = CultureInfo.GetCultureInfo("fr-FR");

            var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

            Assert.Equal(0.5f, emitter.SoftFadeDistance);
            Assert.Equal(0.2f, emitter.Textures[0].Animation.AnimTimeScale);
            Assert.Equal(25.0f, emitter.Emission.EmissionRate.Frames[0].Value);
        }
        finally
        {
            CultureInfo.CurrentCulture = originalCulture;
            CultureInfo.CurrentUICulture = originalUiCulture;
        }
    }

    [Fact]
    public void WriteEmitter_PreservesMultipleTextureSlots()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);
        emitter.Textures.Add(new TextureSlot
        {
            Name = "particles/spark_overlay",
            PatternRepeatHor = 4,
            PatternRepeatVer = 1
        });
        emitter.Textures[1].Animation.Mode = TextureAnimMode.Linear;
        emitter.Textures[1].Animation.AnimIndex.AddRange([3, 2, 1, 0]);
        emitter.Textures[1].Animation.AnimTimeScale = 0.11f;

        var yaml = ParticleYamlWriter.WriteEmitter(emitter);
        var roundTripped = ParticleYamlParser.ParseEmitterFromYaml(yaml);

        Assert.Equal(2, roundTripped.Textures.Count);
        Assert.Equal("particles/spark", roundTripped.Textures[0].Name);
        Assert.Equal("particles/spark_overlay", roundTripped.Textures[1].Name);
        Assert.Equal(4, roundTripped.Textures[1].PatternRepeatHor);
        Assert.Equal(TextureAnimMode.Linear, roundTripped.Textures[1].Animation.Mode);
        Assert.Equal([3, 2, 1, 0], roundTripped.Textures[1].Animation.AnimIndex);
        Assert.Equal(0.11f, roundTripped.Textures[1].Animation.AnimTimeScale);
    }

    [Fact]
    public void ParseEmitter_ParsesEmission()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(EmissionType.OneShot, emitter.Emission.Type);
        Assert.Equal(3.0f, emitter.Emission.EmissionTime);
        Assert.Equal(64, emitter.Emission.MaxParticles);
        Assert.Single(emitter.Emission.EmissionRate.Frames);
        Assert.Equal(25.0f, emitter.Emission.EmissionRate.Frames[0].Value);
    }

    [Fact]
    public void ParseEmitter_ParsesVelocity()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(0.5f, emitter.OmniVelocity.Frames[0].Value);
        Assert.Equal(1.0f, emitter.DirVelocity.Frames[0].Value);
        Assert.Equal(0.0f, emitter.VelocityDir.X);
        Assert.Equal(1.0f, emitter.VelocityDir.Y);
        Assert.Equal(30.0f, emitter.VelocityDirConeDeg);
    }

    [Fact]
    public void ParseEmitter_ParsesLife()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Single(emitter.Life.Frames);
        Assert.Equal(2.0f, emitter.Life.Frames[0].Value);
        Assert.Equal(0.3f, emitter.LifeRandomness);
    }

    [Fact]
    public void ParseEmitter_ParsesColor()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(ColorMode.Constant, emitter.Color.Mode);
        Assert.Equal(1.0f, emitter.Color.Color0.R);
        Assert.Equal(0.8f, emitter.Color.Color0.G);
        Assert.Equal(0.2f, emitter.Color.Color0.B);
    }

    [Fact]
    public void ParseEmitter_ParsesShapeDetails()
    {
        var emitter = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);

        Assert.Equal(0.5f, emitter.ShapeDetails.BaseShape.Hollowness);
        Assert.Equal(360.0f, emitter.ShapeDetails.Arc.ArcWidthDeg);
    }

    [Fact]
    public void ParseEffect_ParsesBasicProperties()
    {
        var effect = ParticleYamlParser.ParseEffectFromYaml(SampleEffectYaml);

        Assert.Equal(2, effect.PreloadCount);
    }

    [Fact]
    public void ParseEffect_ParsesEmitters()
    {
        var effect = ParticleYamlParser.ParseEffectFromYaml(SampleEffectYaml);

        Assert.Equal(2, effect.Emitters.Count);

        Assert.Equal("spark_burst", effect.Emitters[0].EmitterName);
        Assert.Equal(0.5f, effect.Emitters[0].Position.Y);
        Assert.Equal(1.5f, effect.Emitters[0].ParticleScale);

        Assert.Equal("smoke_trail", effect.Emitters[1].EmitterName);
        Assert.Equal(2.0f, effect.Emitters[1].Scale.X);
        Assert.Equal(45.0f, effect.Emitters[1].Rotation.Y);
        Assert.Equal(0.5f, effect.Emitters[1].ReleaseFrame);
    }

    [Fact]
    public void WriteEmitter_RoundTrips()
    {
        var original = ParticleYamlParser.ParseEmitterFromYaml(SampleEmitterYaml);
        var yaml = ParticleYamlWriter.WriteEmitter(original);
        var roundTripped = ParticleYamlParser.ParseEmitterFromYaml(yaml);

        // Verify key properties survived round-trip
        Assert.Equal(original.SoftFadeDistance, roundTripped.SoftFadeDistance);
        Assert.Equal(original.Shape, roundTripped.Shape);
        Assert.Equal(original.Emission.MaxParticles, roundTripped.Emission.MaxParticles);
        Assert.Equal(original.Life.Frames[0].Value, roundTripped.Life.Frames[0].Value);
        Assert.Equal(original.Textures[0].Name, roundTripped.Textures[0].Name);
    }

    [Fact]
    public void WriteEffect_RoundTrips()
    {
        var original = ParticleYamlParser.ParseEffectFromYaml(SampleEffectYaml);
        var yaml = ParticleYamlWriter.WriteEffect(original);
        var roundTripped = ParticleYamlParser.ParseEffectFromYaml(yaml);

        Assert.Equal(original.PreloadCount, roundTripped.PreloadCount);
        Assert.Equal(original.Emitters.Count, roundTripped.Emitters.Count);
        Assert.Equal(original.Emitters[0].EmitterName, roundTripped.Emitters[0].EmitterName);
        Assert.Equal(original.Emitters[0].ParticleScale, roundTripped.Emitters[0].ParticleScale);
    }
}
