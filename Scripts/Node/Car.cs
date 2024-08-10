using Godot;
using System;

public partial class Car : StaticBody3D
{
    public const double acceleration = 1.0; // m/s^2
    public const double maxSpeed = 0.333; // m/s
    public const double stopDeacceleration = 20.0; // m/s^2
    public const double maxSteeringAngle = 30.0; // degree, Maximum steering angle
    public const double wheelBase = 0.122; // m, Distance between front and rear axles
    public const double trackWidth = 0.094; // m, Distance between left and right wheels on the same axle

    private double speed = 0.0;
    private double steeringPercent = 0.0;

    private double speedTarget = 0.0;
    private double steeringPercentTarget = 0.0;

    private CarControllerHandler controllerHandler;

    private SocketMessageWriter socketMessageWriter;
    private float initialRotationDegrees_Y;

    public override void _Ready()
    {
        controllerHandler = new SocketCarController();

        socketMessageWriter = SocketMessageWriter.Instance;
        initialRotationDegrees_Y = RotationDegrees.Y;
    }

    public override void _Process(double delta)
    {
        float angle = initialRotationDegrees_Y - RotationDegrees.Y;
        angle = angle % 360;
        if (angle < 0) { angle += 360; }
        socketMessageWriter.SendGyroBytes(
            BitConverter.GetBytes(angle)
        );

        controllerHandler.Update(this, delta);

        speed += acceleration*delta;
        if (Math.Abs(speed) >= speedTarget) speed = Math.Sign(speed)*speedTarget;

        var newSteeringPercent = steeringPercent + Math.Sign(steeringPercentTarget-steeringPercent)*5.0*delta;
        var minSteeringPercent = Math.Min(steeringPercent, steeringPercentTarget);
        var maxSteeringPercent = Math.Max(steeringPercent, steeringPercentTarget);
        if (minSteeringPercent <= newSteeringPercent && newSteeringPercent <= maxSteeringPercent) {
            steeringPercent = newSteeringPercent;
        } else {
            steeringPercent = steeringPercentTarget;
        }

        if (Math.Abs(steeringPercent) >= 0.0001) {
            double turningRadius = GetTurningRadius();
            Utils.Pivot(this, ToGlobal(new Vector3(-(float)turningRadius, 0, -(float)(wheelBase/2.0))), Vector3.Up, -(float)(speed/turningRadius*delta));
        } else {
            Position += (float)(speed*delta)*GlobalTransform.Basis.Z;
        }

    }

    private double GetTurningRadius()
    {
        return wheelBase / Math.Tan(Mathf.DegToRad(steeringPercent*maxSteeringAngle));
    }

    public void SetSpeed(float speed)
    {
        speedTarget = speed;
        if (speedTarget > maxSpeed) speedTarget = maxSpeed;
    }

    public void SetSteeringPercent(float percent)
    {
        steeringPercentTarget = percent;
    }
}
