using Godot;
using System.Threading.Tasks;

public partial class PythonExecutor : Node
{
    public override async void _Ready()
    {
        string interpreterPath = ProjectSettings.GlobalizePath("res://PythonScripts/venv/Scripts/python.exe");
        string pythonFilePath = ProjectSettings.GlobalizePath("res://PythonScripts/SocketClient.py");

        int err = await ExecutePythonFileAsync(interpreterPath, pythonFilePath);
        GD.Print(err);
    }

    private Task<int> ExecutePythonFileAsync(string interpreterPath, string pythonFilePath)
    {
        return Task.Run(() => 
        {
            var err = OS.Execute(interpreterPath, new string[]{pythonFilePath});
            return err;
        });
    }
}
