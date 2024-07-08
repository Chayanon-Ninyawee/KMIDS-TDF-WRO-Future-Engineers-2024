using Godot;
using System;

public partial class SocketMessageWriter: Node
{
    private static SocketMessageWriter _instance;
    public static SocketMessageWriter Instance => _instance;

    private const int ImageBytesLength = 854 * 480 * 3;
    private const int UltrasonicBytesLength = 4;

    private SocketServer socketServer;

    // Message Format: image[2764800], frontUltrasonic[4], backUltrasonic[4], leftUltrasonic[4], rightUltrasonic[4]
    private byte[] message = new byte[ImageBytesLength + UltrasonicBytesLength*4];

    public void SendImageBytes(byte[] imageBytes)
    {
        if (imageBytes.Length != ImageBytesLength)
        {
            throw new ArgumentException("Image bytes length must be " + ImageBytesLength, nameof(imageBytes));
        }
        Array.Copy(imageBytes, 0, message, 0, ImageBytesLength);
    }

    public void SendFrontUltrasonicBytes(byte[] frontUltrasonic)
    {
        if (frontUltrasonic.Length != UltrasonicBytesLength)
        {
            throw new ArgumentException("Ultrasonic data length must be " + UltrasonicBytesLength + " bytes");
        }
        
        Array.Copy(frontUltrasonic, 0, message, ImageBytesLength + UltrasonicBytesLength*0, UltrasonicBytesLength);
    }

    public void SendBackUltrasonicBytes(byte[] backUltrasonic)
    {
        if (backUltrasonic.Length != UltrasonicBytesLength) 
        {
            throw new ArgumentException("Ultrasonic data length must be " + UltrasonicBytesLength + " bytes");
        }
        
        Array.Copy(backUltrasonic, 0, message, ImageBytesLength + UltrasonicBytesLength*1, UltrasonicBytesLength);
    }

    public void SendLeftUltrasonicBytes(byte[] leftUltrasonic)
    {
        if (leftUltrasonic.Length != UltrasonicBytesLength) 
        {
            throw new ArgumentException("Ultrasonic data length must be " + UltrasonicBytesLength + " bytes");
        }
        
        Array.Copy(leftUltrasonic, 0, message, ImageBytesLength + UltrasonicBytesLength*2, UltrasonicBytesLength);
    }

    public void SendRightUltrasonicBytes(byte[] rightUltrasonic) {
        if (rightUltrasonic.Length != UltrasonicBytesLength) 
        {
            throw new ArgumentException("Ultrasonic data length must be " + UltrasonicBytesLength + " bytes");
        }
        
        Array.Copy(rightUltrasonic, 0, message, ImageBytesLength + UltrasonicBytesLength*3, UltrasonicBytesLength);
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
    }

    public override void _Process(double delta)
    {
        socketServer.BroadcastMessage(message);
    }
}
