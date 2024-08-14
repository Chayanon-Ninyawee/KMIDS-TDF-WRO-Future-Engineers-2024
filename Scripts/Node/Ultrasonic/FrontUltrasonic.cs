using System;
using Godot;

public partial class FrontUltrasonic : RayCast3D
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
            socketMessageWriter.SendFrontUltrasonicBytes(
                BitConverter.GetBytes(GetCollisionPoint().DistanceTo(GlobalPosition))
            );
        }
    }
}