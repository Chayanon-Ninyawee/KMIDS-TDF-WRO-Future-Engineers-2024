# WRO 2024 Future Engineer
Team: KMIDS Thermodynamically Favorable (KMIDS TDF)

This is a repository for KMIDS Thermodynamically Favorable this project, we aim to design an autonomous robot that can navigate around a field with various rules and restrictions depending on the rounds. In this repository you will find details about our robot: function, design, and reflection from our journey.

![New robot](https://github.com/user-attachments/assets/a415d5c4-0dc0-44ec-8452-be225b8329a5)
![Team member pic](https://github.com/user-attachments/assets/9d38cc9d-c21b-467a-9a42-78c8fbeeb7f8)

**Member**
- **Left:** Natthapon Itthisathidkulchai (Pao)
- **Middle:** Chayanon Ninyawee (Garfield)
- **Right:** Pannawit Phruithithada (Prame)

## Content
- [RaspberryPi5](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/RaspberryPi5) Contains our code for RaspberryPi5 and setup.<!--TODO at link-->
- [RaspberryPiPico](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/RaspberryPiPico) Contains our code for RaspberryPiPico and setup.<!--TODO at link-->
- [KMIDS-TDF-WRO2024-FreeCAD](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/KMIDS-TDF-WRO2024-FreeCAD) Contains robot's 3D model and electronics.
- [Mobility Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/blob/main/Documentation/Mobility_Management/Mobility_Management.md) Contains robot's design and mechanism.
- [Obstacle Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Obstacle%20Management) Contains how we deal with the obstacle challenges.
- [Power and Senses Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Power%20and%20Senses%20Management) Contains how we manage our electronics and various sensors.
- [Robot Picture](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Robot%20Picture) Contains picture of the robot, both in physical and digital 3D model from FreeCad.
- [Specification](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Specification) Contains the specifications of various electronics and parts uses to built this robot.
- [Team Member Picture](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Team%20Picture) Contains our team's picture.
- [Setup Freecad](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/main/Documentation/Setup%20freecad) Contains how to install and run Freecad.

## Performance video
Our video demonstrating our robot can be found [here](https://youtu.be/9CfIpZZZoUU?si=juIyMaRLtTwPCz_5)

## 3D Model

Our 3D model is made and designed in FreeCAD, specificly the [realthunder](https://github.com/realthunder/FreeCAD/releases) version which is free open source CAD software that allows a wide range of developers to access the field of 3D modeling. Our 3D model can be viewed in `KMIDS-TDF-WRO2024-FreeCAD` when opening `MainAssembly`. We use `creality Ender-3 V3` to print the chassis of our robot but other printer should also work just fine.

## Design Journey
When designing the robot, the first thing that we ruminated on is how the robot will park. This is a very important step to consider because we can choose to have it park back first into the wall or have it do a parallel park. The second thing that we consider is the turning angle of the robot: how to optimize it so that it has the maximum turn angle to help avoid obstacles. Then comes the process of designing the chassis and where to put various electronics that we want on it. A major problem in this step is the limitation of our RPLIDAR C1, which can only be at a certain height; otherwise, it won't detect the wall. The last and most annoying part for us is the prototyping phase, where we need to keep printing and readjusting the model because the actual 3D parts can deviate from the original design.