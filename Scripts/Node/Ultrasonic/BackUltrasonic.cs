using System;
using Godot;

public partial class BackUltrasonic : RayCast3D
{
    private SocketMessageWriter socketMessageWriter;

    public override void _Ready()
    {
        socketMessageWriter = SocketMessageWriter.Instance;
    }

    public override void _Process(double delta)
    {
        if (IsColliding())
        {
            socketMessageWriter.SendBackUltrasonicBytes(
                BitConverter.GetBytes(ToLocal(GetCollisionPoint()).DistanceTo(Position))
            );
        }
    }
}