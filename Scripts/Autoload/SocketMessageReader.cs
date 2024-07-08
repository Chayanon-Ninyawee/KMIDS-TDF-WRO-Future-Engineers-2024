using Godot;
using System;
using System.Linq;
using System.Net.Sockets;

public partial class SocketMessageReader: Node
{
    private static SocketMessageReader _instance;
    public static SocketMessageReader Instance => _instance;

    private SocketServer socketServer;

    private float accelerationPercent = 0.0f;
    public float AccelerationPercent {
        get { return accelerationPercent; }
    }

    private float steeringPercent = 0.0f;
    public float SteeringPercent {
        get { return steeringPercent; }
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
        socketServer = SocketServerInstances.Instance.SocketServer;
        socketServer.OnMessageReceived += HandleMessageReceived;
    }

    public override void _ExitTree()
    {
        socketServer.OnMessageReceived -= HandleMessageReceived;
    }

    private void HandleMessageReceived(TcpClient client, byte[] message)
    {
        // Message Format: accelerationPercent[4], steeringPercent[4]
        if (message.Length != 8) return;

        byte[] accelerationPercentBytes = message.Take(message.Length/2).ToArray();
        byte[] steeringPercentBytes = message.Skip(message.Length/2).ToArray();

        accelerationPercent = BitConverter.ToSingle(accelerationPercentBytes, 0);
        steeringPercent = BitConverter.ToSingle(steeringPercentBytes, 0);
    }
}
