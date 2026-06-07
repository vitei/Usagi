// Usagi.ToolCore - Particle Editor Support
// Simple vector types for particle properties

namespace Usagi.ToolCore.Particles;

/// <summary>
/// 3D vector for particle positions, velocities, scales, etc.
/// </summary>
public struct Vec3
{
    public float X { get; set; }
    public float Y { get; set; }
    public float Z { get; set; }

    public Vec3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public static Vec3 Zero => new(0, 0, 0);
    public static Vec3 One => new(1, 1, 1);
    public static Vec3 Up => new(0, 1, 0);
    public static Vec3 Down => new(0, -1, 0);
}

/// <summary>
/// 2D vector for UV coordinates, particle centers, etc.
/// </summary>
public struct Vec2
{
    public float X { get; set; }
    public float Y { get; set; }

    public Vec2(float x, float y)
    {
        X = x;
        Y = y;
    }

    public static Vec2 Zero => new(0, 0);
    public static Vec2 Half => new(0.5f, 0.5f);
}

/// <summary>
/// RGBA color for particle colors.
/// </summary>
public struct Color4
{
    public float R { get; set; }
    public float G { get; set; }
    public float B { get; set; }
    public float A { get; set; }

    public Color4(float r, float g, float b, float a = 1.0f)
    {
        R = r;
        G = g;
        B = b;
        A = a;
    }

    public static Color4 White => new(1, 1, 1, 1);
    public static Color4 Black => new(0, 0, 0, 1);
    public static Color4 Clear => new(0, 0, 0, 0);
}
