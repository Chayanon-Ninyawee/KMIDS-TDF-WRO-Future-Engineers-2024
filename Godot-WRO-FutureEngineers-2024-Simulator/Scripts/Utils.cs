using Godot;

public static class Utils
{
    public static void Pivot(Node3D node3d, Vector3 pivot_point, Vector3 axis, float angle)
    {
        node3d.Position = (node3d.Position - pivot_point).Rotated(axis, angle) + pivot_point;
        node3d.Rotate(axis, angle);
    }
}