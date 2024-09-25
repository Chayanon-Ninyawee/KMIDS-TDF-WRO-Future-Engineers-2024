public class SocketCarController : CarControllerHandler
{
    private SocketMessageReader socketMessageReader;

    public SocketCarController() {
        socketMessageReader = SocketMessageReader.Instance;
    }

    public void Update(Car car, double delta)
    {
        car.SetSpeed(
            socketMessageReader.SpeedTarget
        );

        car.SetSteeringPercent(
            socketMessageReader.SteeringPercent
        );
    }
}
