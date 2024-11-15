# WRO-2024-Future_Engineer
Team: KMIDS Thermodynamically Favorable (KMIDS TDF)

This is a repository for KMIDS Thermodynamically Favorable this project we aim to design an 
autonomous robot that can navigate around a field with various rules and restrictions depending 
on the rounds. In this repository you will find details about our robot: function, design and
reflection from our journey.

![New robot](https://github.com/user-attachments/assets/a415d5c4-0dc0-44ec-8452-be225b8329a5)
![Team pic specific](https://github.com/user-attachments/assets/9d38cc9d-c21b-467a-9a42-78c8fbeeb7f8)

**Member**
- **Left:** Natthapon Itthisathidkulchai (Pao)
- **Middle:** Chayanon Ninyawee (Garfield)
- **Right:** Pannawit Phruithithada (Prame)

## Content
- [Code](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/42423e1957acbe372c20b5e2d8ece8258ac3caf3/Code) Contains our code for the robot.
- [KMIDS-TDF-WRO2024-FreeCAD](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/42423e1957acbe372c20b5e2d8ece8258ac3caf3/KMIDS-TDF-WRO2024-FreeCAD) Contains 3D model files of our robot and it's electronics.
- [Mobility Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/141529bab1eff6fb5a7f057b7a112ee0c0a9d1bd/Documentation/Mobility%20Management) Contains how we design our robot.
- [Obstacle Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/141529bab1eff6fb5a7f057b7a112ee0c0a9d1bd/Documentation/Obstacle%20Management) Contains how we deal with the challenges that each rounds present us.
- [Power and Senses Management](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/59d9f73f7ef363b6e41c1c2cada7da4087ea8578/Documentation/Power%20and%20Senses%20Management) Contains how we manages our electronics and various sensors.
- [Robot Picture](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/141529bab1eff6fb5a7f057b7a112ee0c0a9d1bd/Documentation/Robot%20Picture) Contains our picture of the robot, both real and from the 3D models.
- [Specification](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/141529bab1eff6fb5a7f057b7a112ee0c0a9d1bd/Documentation/Specification) Contains the specifications of various electronics and parts uses to built this robot.
- [Team Picture](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024/tree/141529bab1eff6fb5a7f057b7a112ee0c0a9d1bd/Documentation/Team%20Picture) Contains our team's picture.

## Performance video
our video demonstrating our robot can be found [here](https://youtu.be/9CfIpZZZoUU?si=juIyMaRLtTwPCz_5)

## 3D_Model

Our 3D model are made and design in FreeCAD specificly the [realthunder](https://github.com/realthunder/FreeCAD/releases)
version which is a free open source CAD software that allow a wide range of developer to access the field 
of 3D modeling. Our 3D models can be view in `KMIDS-TDF-WRO2024-FreeCAD` when openign `MainAssembly`

## Design Journey
When designing the robot, the first thing that we ruminated on is how the robot will park. This is a very
important step to consider because we can choose to have it park back first into the wall or have it do a
paralell park. The secon thing that we consider is the turning angle of the robot: how to optimise it so 
that it have the maximum turn angle to help avoid obstacles. Then comes the process of designing the chassis
and where to put various electronics that we want on it, a major problem in this step is the limitation of
our RPLIDAR C1 that can only be at a certain heign else it won't detect the wall. The last and most annoying
part for us is the prototyping phase where we need to keep printing and readjust the model, because the actual
3D parts can deviate from the original design.
