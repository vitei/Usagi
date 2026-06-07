// Usagi.ToolCore.Tests - Particle Document Tests

using Usagi.ToolCore.Particles;
using Xunit;

namespace Usagi.ToolCore.Tests.Particles;

public class ParticleDocumentTests
{
    [Fact]
    public void EmitterDocument_CreateNew_HasDefaultSettings()
    {
        var doc = EditableEmitterDocument.CreateNew("TestEmitter");

        Assert.Equal("TestEmitter", doc.Emitter.Name);
        Assert.Single(doc.Emitter.Textures);
        Assert.False(doc.IsDirty);
    }

    [Fact]
    public void EmitterDocument_SetMaxParticles_MarksDirty()
    {
        var doc = EditableEmitterDocument.CreateNew();

        doc.SetMaxParticles(256);

        Assert.True(doc.IsDirty);
        Assert.Equal(256, doc.Emitter.Emission.MaxParticles);
    }

    [Fact]
    public void EmitterDocument_SetMaxParticles_CanUndo()
    {
        var doc = EditableEmitterDocument.CreateNew();
        var original = doc.Emitter.Emission.MaxParticles;

        doc.SetMaxParticles(256);
        Assert.Equal(256, doc.Emitter.Emission.MaxParticles);

        doc.History.Undo();
        Assert.Equal(original, doc.Emitter.Emission.MaxParticles);
    }

    [Fact]
    public void EmitterDocument_SetShape_CanUndoRedo()
    {
        var doc = EditableEmitterDocument.CreateNew();

        doc.SetShape(EmitterShape.Sphere);
        Assert.Equal(EmitterShape.Sphere, doc.Emitter.Shape);

        doc.SetShape(EmitterShape.Cone);
        Assert.Equal(EmitterShape.Cone, doc.Emitter.Shape);

        doc.History.Undo();
        Assert.Equal(EmitterShape.Sphere, doc.Emitter.Shape);

        doc.History.Redo();
        Assert.Equal(EmitterShape.Cone, doc.Emitter.Shape);
    }

    [Fact]
    public void EmitterDocument_AddTexture_AddsAndCanUndo()
    {
        var doc = EditableEmitterDocument.CreateNew();
        var initialCount = doc.Emitter.Textures.Count;

        doc.AddTexture("particles/fire");

        Assert.Equal(initialCount + 1, doc.Emitter.Textures.Count);
        Assert.Equal("particles/fire", doc.Emitter.Textures[^1].Name);

        doc.History.Undo();
        Assert.Equal(initialCount, doc.Emitter.Textures.Count);
    }

    [Fact]
    public void EmitterDocument_RemoveTexture_RemovesAndCanUndo()
    {
        var doc = EditableEmitterDocument.CreateNew();
        doc.AddTexture("particles/smoke");
        var count = doc.Emitter.Textures.Count;

        doc.RemoveTexture(count - 1);

        Assert.Equal(count - 1, doc.Emitter.Textures.Count);

        doc.History.Undo();
        Assert.Equal(count, doc.Emitter.Textures.Count);
        Assert.Equal("particles/smoke", doc.Emitter.Textures[^1].Name);
    }

    [Fact]
    public void EffectDocument_CreateNew_HasSingleEmitter()
    {
        var doc = EditableEffectDocument.CreateNew("TestEffect", "test_emitter");

        Assert.Equal("TestEffect", doc.Effect.Name);
        Assert.Single(doc.Effect.Emitters);
        Assert.Equal("test_emitter", doc.Effect.Emitters[0].EmitterName);
    }

    [Fact]
    public void EffectDocument_AddEmitter_AddsAndMarksDirty()
    {
        var doc = EditableEffectDocument.CreateNew("TestEffect", "emitter1");

        var instance = doc.AddEmitter("emitter2");

        Assert.True(doc.IsDirty);
        Assert.Equal(2, doc.Effect.Emitters.Count);
        Assert.Equal("emitter2", instance.EmitterName);
    }

    [Fact]
    public void EffectDocument_RemoveEmitter_RemovesAndCanUndo()
    {
        var doc = EditableEffectDocument.CreateNew("TestEffect", "emitter1");
        doc.AddEmitter("emitter2");
        var instance = doc.Effect.Emitters[1];

        doc.RemoveEmitter(instance);

        Assert.Single(doc.Effect.Emitters);

        doc.History.Undo();
        Assert.Equal(2, doc.Effect.Emitters.Count);
    }

    [Fact]
    public void EffectDocument_SetEmitterPosition_CanUndo()
    {
        var doc = EditableEffectDocument.CreateNew("TestEffect", "emitter1");
        var instance = doc.Effect.Emitters[0];
        var original = instance.Position;

        var newPos = new Vec3(1, 2, 3);
        doc.SetEmitterPosition(instance, newPos);

        Assert.Equal(1.0f, instance.Position.X);
        Assert.Equal(2.0f, instance.Position.Y);
        Assert.Equal(3.0f, instance.Position.Z);

        doc.History.Undo();
        Assert.Equal(original.X, instance.Position.X);
        Assert.Equal(original.Y, instance.Position.Y);
        Assert.Equal(original.Z, instance.Position.Z);
    }

    [Fact]
    public void EffectDocument_SetPreloadCount_CanUndo()
    {
        var doc = EditableEffectDocument.CreateNew("TestEffect", "emitter1");
        var original = doc.Effect.PreloadCount;

        doc.SetPreloadCount(5);
        Assert.Equal(5, doc.Effect.PreloadCount);

        doc.History.Undo();
        Assert.Equal(original, doc.Effect.PreloadCount);
    }
}
