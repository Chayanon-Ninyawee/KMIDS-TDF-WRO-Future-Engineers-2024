using Godot;

public partial class SubViewportCapture : SubViewport
{
    private SocketMessageWriter socketMessageWriter;

    private Node3D parent;
    private Camera3D camera3D;
    private Node3D cameraNode;

    public override void _Ready()
    {
        socketMessageWriter = SocketMessageWriter.Instance;

        parent = GetParent<Node3D>();
        camera3D = GetCamera3D();
        cameraNode = GetChild<Node3D>(0);
    }

    public override void _Process(double delta)
    {
        cameraNode.Position = parent.Position;
        cameraNode.Rotation = parent.Rotation;

        socketMessageWriter.SendImageBytes(
            GetViewport().GetTexture().GetImage().GetData()
        );
    }
}
