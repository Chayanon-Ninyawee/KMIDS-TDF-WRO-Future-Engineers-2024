using System;
using Godot;

public partial class RightUltrasonic : RayCast3D
{
    private Node3D parent;
    
    private SocketMessageWriter socketMessageWriter;

    public override void _Ready()
    {
        parent = GetParent<Node3D>();

        socketMessageWriter = SocketMessageWriter.Instance;
    }

    public override void _Process(double delta)
    {
        if (IsColliding())
        {
            socketMessageWriter.SendRightUltrasonicBytes(
                BitConverter.GetBytes(GetCollisionPoint().DistanceTo(parent.Position))
            );
        }
    }
}