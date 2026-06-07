// Usagi.ToolCore - Particle Editor Support
// Animated values with keyframes

namespace Usagi.ToolCore.Particles;

/// <summary>
/// A single keyframe in an animation curve.
/// </summary>
public sealed class Keyframe
{
    public float TimeIndex { get; set; }
    public float Value { get; set; }

    public Keyframe() { }

    public Keyframe(float time, float value)
    {
        TimeIndex = time;
        Value = value;
    }
}

/// <summary>
/// An animated float value with keyframes.
/// </summary>
public sealed class AnimatedFloat
{
    public List<Keyframe> Frames { get; } = [];

    public AnimatedFloat() { }

    public AnimatedFloat(float constantValue)
    {
        Frames.Add(new Keyframe(0, constantValue));
    }

    /// <summary>
    /// Gets the value at a normalized time (0-1).
    /// </summary>
    public float GetValue(float normalizedTime)
    {
        if (Frames.Count == 0) return 0;
        if (Frames.Count == 1) return Frames[0].Value;

        // Find surrounding keyframes and lerp
        for (int i = 0; i < Frames.Count - 1; i++)
        {
            if (normalizedTime >= Frames[i].TimeIndex && normalizedTime <= Frames[i + 1].TimeIndex)
            {
                float t = (normalizedTime - Frames[i].TimeIndex) /
                          (Frames[i + 1].TimeIndex - Frames[i].TimeIndex);
                return Frames[i].Value + (Frames[i + 1].Value - Frames[i].Value) * t;
            }
        }

        return Frames[^1].Value;
    }
}
