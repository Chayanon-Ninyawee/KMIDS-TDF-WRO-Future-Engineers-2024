using Godot;

public partial class SocketServerInstances: Node
{
    private static SocketServerInstances _instance;
    public static SocketServerInstances Instance => _instance;


    private readonly SocketServer socketServer = new SocketServer("127.0.0.1", 12345);
    public SocketServer SocketServer
    {
        get { return socketServer; }   // get method
    }
    
    // Use _EnterTree to make sure the Singleton instance is avaiable in _Ready()
    public override void _EnterTree()
    {
        if(_instance != null){
            this.QueueFree(); // The Singletone is already loaded, kill this instance
        }
        _instance = this;
    }

    public override void _Ready()
    {
        socketServer.StartServer();
    }

    public override void _ExitTree()
    {
        socketServer.StopServer();
    }
}