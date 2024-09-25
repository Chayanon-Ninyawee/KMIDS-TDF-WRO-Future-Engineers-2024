using Godot;
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;

public class SocketServer
{
    private readonly IPAddress ipAddress;
    private readonly int port;

    private TcpListener _server;
    private bool _isRunning;
    private List<TcpClient> _clients = new List<TcpClient>();

    // Define a delegate for the message received event
    public delegate void MessageReceivedHandler(TcpClient client, byte[] message);
    // Define the event using the delegate
    public event MessageReceivedHandler OnMessageReceived;


    public SocketServer(String ipAddress, int port)
    {
        this.ipAddress = IPAddress.Parse(ipAddress);
        this.port = port;
    }


    public void StartServer()
    {
        _isRunning = true;
        Task.Run(() => ListenForClients());
    }

    public void StopServer()
    {
        _isRunning = false;
        _server.Stop();
    }

    private async void ListenForClients()
    {
        _server = new TcpListener(ipAddress, port);
        _server.Start();
        GD.Print("Server started on " + ipAddress + ":" + port);

        while (_isRunning)
        {
            try
            {
                TcpClient client = await _server.AcceptTcpClientAsync();
                GD.Print("Client connected");
                _clients.Add(client);
                _ = Task.Run(() => HandleClient(client));
            }
            catch (Exception ex)
            {
                GD.PrintErr($"Error accepting client: {ex.Message}");
            }
        }
    }

    private async void HandleClient(TcpClient client)
    {
        NetworkStream stream = client.GetStream();
        byte[] buffer = new byte[1024];

        while (_isRunning && client.Connected)
        {
            try
            {
                int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
                if (bytesRead == 0)
                {
                    // Client disconnected
                    break;
                }

                byte[] receivedData = new byte[bytesRead];
                Array.Copy(buffer, receivedData, bytesRead);
                GD.Print($"Received {bytesRead} bytes from client");

                // Trigger the event
                OnMessageReceived?.Invoke(client, receivedData);
            }
            catch (Exception ex)
            {
                GD.PrintErr($"Error handling client: {ex.Message}");
                break;
            }
        }

        _clients.Remove(client);
        client.Close();
        GD.Print("Client disconnected");
    }

    public static async void SendMessageToClient(TcpClient client, byte[] message)
    {
        try
        {
            NetworkStream stream = client.GetStream();
            await stream.WriteAsync(message, 0, message.Length);
            GD.Print($"Sent {message.Length} bytes to client");
        }
        catch (Exception ex)
        {
            GD.PrintErr($"Error sending message to client: {ex.Message}");
        }
    }

    public void BroadcastMessage(byte[] message)
    {
        foreach (var client in _clients)
        {
            SendMessageToClient(client, message);
        }
    }

    // Add a method to get a specific client
    public TcpClient GetClient(int index)
    {
        if (index < 0 || index >= _clients.Count)
        {
            GD.PrintErr("Invalid client index");
            return null;
        }
        return _clients[index];
    }

    // Add a property to get the number of clients
    public int ClientCount => _clients.Count;
}
