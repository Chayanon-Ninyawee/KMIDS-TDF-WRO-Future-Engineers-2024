public class SocketCarController : CarControllerHandler
{
    private SocketMessageReader socketMessageReader;

    public SocketCarController() {
        socketMessageReader = SocketMessageReader.Instance;
    }

    public void Update(Car car, double delta)
    {
        car.Accelerate(
            socketMessageReader.AccelerationPercent
        );

        car.SetSteeringPercent(
            socketMessageReader.SteeringPercent
        );
    }
}
