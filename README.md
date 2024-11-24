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
-[3D Model]() Contains the process of developing and modeling the chassis.

## Performance video
Our video demonstrating our robot can be found [here](https://youtu.be/9CfIpZZZoUU?si=juIyMaRLtTwPCz_5)
## Design Journey
When designing the robot, our first consideration was its parking strategy. This is a crucial decision, as we had to choose between having the robot reverse into a wall or perform a parallel parking maneuver. Next, we focused on optimizing the robot's turning angle to ensure it could avoid obstacles effectively by maximizing its range of motion. Following that, we moved on to designing the chassis and determining the optimal placement for the various electronic components. A significant challenge during this stage was the limitation of our RPLIDAR C1 sensor, which must be positioned at a precise height to detect the wall properly. Finally, the prototyping phase proved to be the most challenging. We repeatedly faced issues with 3D printing, as the actual printed parts often deviated from the original design, requiring constant adjustments and refinements to achieve the desired results.