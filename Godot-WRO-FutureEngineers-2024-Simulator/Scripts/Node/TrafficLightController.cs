using System.Collections.Generic;
using Godot;

public partial class TrafficLightController : Node
{
    private List<Node3D> redTrafficLight = new();
    private List<Node3D> greenTrafficLight = new();

    public override void _Ready()
    {
        for (int i = 0; i < 24; i++) {
            redTrafficLight.Add(GetNode<Node3D>("RedTrafficLight"+(i+1)));
            greenTrafficLight.Add(GetNode<Node3D>("GreenTrafficLight"+(i+1)));
        }

        for (int i = 0; i < 24; i++) {

            DisableCollisionIfNotVisiable(redTrafficLight[i]);
            DisableCollisionIfNotVisiable(greenTrafficLight[i]);
        }
    }

    private void HideAndDisableCollision(Node3D trafficLight)
    {
        trafficLight.Visible = false;
        trafficLight.GetNode<CollisionShape3D>("CollisionShape3D").SetDeferred("disabled", true);
    }

    private void ShowAndEnableCollision(Node3D trafficLight)
    {
        trafficLight.Visible = true;
        trafficLight.GetNode<CollisionShape3D>("CollisionShape3D").SetDeferred("disabled", false);
    }

    private void DisableCollisionIfNotVisiable(Node3D trafficLight)
    {
        if (!trafficLight.Visible) {
            trafficLight.GetNode<CollisionShape3D>("CollisionShape3D").SetDeferred("disabled", true);
        }
    }
}
