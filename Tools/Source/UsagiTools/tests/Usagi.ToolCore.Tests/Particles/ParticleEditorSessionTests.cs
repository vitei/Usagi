using Usagi.ToolCore.Particles;
using Xunit;

namespace Usagi.ToolCore.Tests.Particles;

public sealed class ParticleEditorSessionTests
{
    [Fact]
    public void OpenEmitterBuildsSummaryAndTextureViews()
    {
        using var fixture = ParticleFixture.Create();

        var session = ParticleEditorSession.OpenEmitter(fixture.EmitterPath);

        var summary = session.GetEmitterSummary();
        Assert.Equal("multi_texture_slots", summary.Name);
        Assert.Equal(2, summary.TextureCount);
        Assert.Equal(64, summary.MaxParticles);

        var textures = session.GetTextures();
        Assert.Equal(["particles/base", "particles/overlay"], textures.Select(texture => texture.Name));
        Assert.Equal(TextureAnimMode.Linear, textures[1].AnimationMode);
        Assert.Equal([0, 1, 2, 3], textures[1].AnimationIndices);
    }

    [Fact]
    public void EmitterEditsUseCommandHistoryAndSaveRoundTrips()
    {
        using var fixture = ParticleFixture.Create();
        var session = ParticleEditorSession.OpenEmitter(fixture.EmitterPath);

        session.SetMaxParticles(128);
        session.SetEmitterTexture(1, "particles/overlay_bright");

        Assert.True(session.IsDirty);
        Assert.Equal(128, session.GetEmitterSummary().MaxParticles);
        Assert.Equal("particles/overlay_bright", session.GetTextures()[1].Name);

        session.EmitterDocument!.History.Undo();
        Assert.Equal("particles/overlay", session.GetTextures()[1].Name);
        session.EmitterDocument.History.Redo();

        session.SaveAll();
        Assert.False(session.IsDirty);

        var reloaded = ParticleEditorSession.OpenEmitter(fixture.EmitterPath);
        Assert.Equal(128, reloaded.GetEmitterSummary().MaxParticles);
        Assert.Equal("particles/overlay_bright", reloaded.GetTextures()[1].Name);
    }

    [Fact]
    public void OpenEffectWithEmittersBuildsEffectAndEmitterViews()
    {
        using var fixture = ParticleFixture.Create();

        var session = ParticleEditorSession.OpenEffectWithEmitters(fixture.EffectPath, fixture.EmitterDirectory);

        Assert.True(session.HasEffect);
        Assert.True(session.HasEmitter);
        Assert.Equal(1, session.GetEffectSummary().EmitterCount);
        Assert.Equal("multi_texture_slots", session.GetEmitterInstances()[0].EmitterName);
        Assert.Equal(2, session.GetTextures().Count);
    }

    [Fact]
    public void EffectEditsUseCommandHistoryAndConversionPlans()
    {
        using var fixture = ParticleFixture.Create();
        var session = ParticleEditorSession.OpenEffectWithEmitters(fixture.EffectPath, fixture.EmitterDirectory);

        session.SetPreloadCount(3);
        session.SetEffectEmitterPosition(0, new Vec3(1.0f, 2.0f, 3.0f));
        session.AddEffectEmitter("secondary");

        Assert.True(session.IsDirty);
        Assert.Equal(2, session.GetEffectSummary().EmitterCount);
        Assert.Equal(1.0f, session.GetEmitterInstances()[0].Position.X);

        session.EffectDocument!.History.Undo();
        Assert.Single(session.GetEmitterInstances());
        session.EffectDocument.History.Redo();

        var emitterPlan = session.GetEmitterConversionPlan(fixture.ProjectRoot);
        var effectPlan = session.GetEffectConversionPlan(fixture.ProjectRoot);

        Assert.EndsWith("multi_texture_slots.pem", emitterPlan.OutputPath, StringComparison.Ordinal);
        Assert.EndsWith("multi_texture_slots.pfx", effectPlan.OutputPath, StringComparison.Ordinal);
        Assert.Contains("yml2vpb.rb", emitterPlan.ScriptPath, StringComparison.Ordinal);
        Assert.Contains(fixture.EmitterPath, emitterPlan.Arguments);
    }

    private sealed class ParticleFixture : IDisposable
    {
        private readonly DirectoryInfo _root;

        private ParticleFixture(DirectoryInfo root, string emitterDirectory, string emitterPath, string effectPath)
        {
            _root = root;
            ProjectRoot = root.FullName;
            EmitterDirectory = emitterDirectory;
            EmitterPath = emitterPath;
            EffectPath = effectPath;
        }

        public string ProjectRoot { get; }
        public string EmitterDirectory { get; }
        public string EmitterPath { get; }
        public string EffectPath { get; }

        public static ParticleFixture Create()
        {
            var root = Directory.CreateTempSubdirectory("usagi-particle-session-");
            var emitterDirectory = Directory.CreateDirectory(Path.Combine(root.FullName, "Data", "Particle", "Emitters"));
            var effectDirectory = Directory.CreateDirectory(Path.Combine(root.FullName, "Data", "Particle", "Effects"));
            Directory.CreateDirectory(Path.Combine(root.FullName, "Tools", "ruby"));

            var emitterPath = Path.Combine(emitterDirectory.FullName, "multi_texture_slots.yml");
            File.WriteAllText(emitterPath, """
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
    - name: particles/base
      uPatternRepeatHor: 1
      uPatternRepeatVer: 1
      textureAnim:
        eTexMode: 0
        bRandomOffset: false
        animIndex: []
        fAnimTimeScale: 1.0
    - name: particles/overlay
      uPatternRepeatHor: 2
      uPatternRepeatVer: 2
      textureAnim:
        eTexMode: 1
        bRandomOffset: false
        animIndex: [0, 1, 2, 3]
        fAnimTimeScale: 0.25
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
  fPositionRandomness: 0.0
  omniVelocity:
    frames:
      - fTimeIndex: 0.0
        fValue: 0.0
  dirVelocity:
    frames:
      - fTimeIndex: 0.0
        fValue: 0.0
  vVelocityDir:
    x: 0.0
    y: 1.0
    z: 0.0
  fVelocityDirConeDeg: 0.0
  fSpeedRandomness: 0.0
  bInheritVelocity: false
  bLocalEffect: true
  bCPUPositionUpdate: false
  fDrag: 0.0
  fGravityStrength: 0.0
  vGravityDir:
    x: 0.0
    y: -1.0
    z: 0.0
  eParticleType: 0
  life:
    frames:
      - fTimeIndex: 0.0
        fValue: 2.0
  fLifeRandomness: 0.0
  vParticleCenter:
    x: 0.5
    y: 0.5
  particleColor:
    eColorMode: 0
    cColor0:
      m_fR: 1.0
      m_fG: 1.0
      m_fB: 1.0
      m_fA: 1.0
    cColor1:
      m_fR: 1.0
      m_fG: 1.0
      m_fB: 1.0
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
    fOutStartTiming: 0.9
  particleRotation:
    fBaseRotation: 0.0
    fRandomise: 0.0
    fSpeed: 0.0
    fSpeedRandomise: 0.0
  particleScale:
    standardValue:
      frames:
        - fTimeIndex: 0.0
          fValue: 1.0
    fRandomness: 0.0
    fInitial: 1.0
    fIntermediate: 1.0
    fEnding: 1.0
    fBeginScaleIn: 0.0
    fStartScaleOut: 1.0
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
    fHollowness: 0.0
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
""");

            var effectPath = Path.Combine(effectDirectory.FullName, "multi_texture_slots.yml");
            File.WriteAllText(effectPath, """
Particles.EffectGroup:
  emitters:
    - emitterName: multi_texture_slots
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
      fParticleScale: 1.0
      fReleaseFrame: 0.0
  uPreloadCount: 0
""");

            return new ParticleFixture(root, emitterDirectory.FullName, emitterPath, effectPath);
        }

        public void Dispose()
        {
            _root.Delete(recursive: true);
        }
    }
}
