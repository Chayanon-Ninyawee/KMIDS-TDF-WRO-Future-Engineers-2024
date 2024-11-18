## Power and Senses Management
### Power Management
The robot is powered by 2 lithium-ion batteries, which are connected to the Raspberry Pi 5.
with the help of an uninterruptable power supply that maintains a steady connection between them.
The lithium-ion batteries provide us with a reusable power source instead of a traditional battery.
which is great as this is more sustainable.

### Sensor Management
**1. RPlidar C1**
The RPLidar C1 is a great alternative to normal sensors (ultrasonics, infrared) as it has the
capabilities to take 360-degree samples, which centralize the space for this sensor to one area.
This sensor is positioned at the center on top of the Raspberry Pi 5 and is the highest point.
of the robot.


**2. Light sensor TC01**
This light sensor detects colors on the field to help guide the robot in turning and rounding.
corners. This sensor is positioned at the bottom of the robot to capture the orange and blue
lines that are present at each corner of the arena. If the sensor detects the blue line first, then
The robot will make a right turn, and if it detects orange first, then it will make a left turn.

**3. BNO055**
The BNO055 provides multiple features to the robot; it can detect the acceleration of the robot.
and also it's dirention as it has a gyro sensor built into it, which helps the robot navigates
the arena as it has access to more information on itself. The BNO055 is positioned on top of
The Raspberry Pi Pico will conserve spaces and ease of connection.

**4. Fish-eye lens camera**
The fish-eye lens camera provides the robot with a wide view of the arena at the front of the
robot, which helps in better obstacle detection and recognition. The camera is connected directly.
to the Raspberry Pi 5 as it's the main processor of the image and is positioned at the front to
see the obstacle up ahead of it. The camera is not placed higher in the robot because, in doing
so will hinder the abilities of the RPLidar C1, which is at the top.

## Wiring Diagram

![circuit](https://github.com/user-attachments/assets/7a223edd-bf7f-4c09-91c3-a9a39c628697)