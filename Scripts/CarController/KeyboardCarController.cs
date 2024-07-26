using Godot;

public class KeyboardCarController : CarControllerHandler
{
    public void Update(Car car, double delta)
    {
        if (Input.IsActionPressed("accelerate"))
        {
            car.SetSpeed(99999.0f); // Full acceleration
        }
        else if (Input.IsActionPressed("brake"))
        {
            car.SetSpeed(-99999.0f); // Full reverse
        }
        else
        {
            car.SetSpeed(0.0f); // Stop
        }

        if (Input.IsActionPressed("left"))
        {
            car.SetSteeringPercent(-1.0f); // Full left
        }
        else if (Input.IsActionPressed("right"))
        {
            car.SetSteeringPercent(1.0f); // Full right
        }
        else
        {
            car.SetSteeringPercent(0.0f); // Straighten wheels
        }
    }
}
