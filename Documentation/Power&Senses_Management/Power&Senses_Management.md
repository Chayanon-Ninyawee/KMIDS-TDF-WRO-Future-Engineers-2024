## Power and Senses Management
### Power Management
The robot is powered by 2 lithium ion batteries which is connected to the raspberry pi 5
with the help of an uninterruptable power supply which maintain a steady connection between them.
The lithium ion batteries provide us with a reusable power source instead of a traditional batteries
which is great as this is more sustainable.

### Sensor Management
**1. RPlidar C1**
The RPLidar C1 is a great alternative to normal sensors (ultrasonics, infrared) as it have the
capabitities to take a 360 degree samples which cetralize the space for this sensor to one area.
This sensor is positioned at the center on top of the Raspberry pi 5 and is the highest point
of the robot.


**2. Light sensor TC01**
This light sensor detects colors on the field to help guide the robot in turning and rounding
corners. This sensor is positioned at the bottom of the robot to captured the orange and blue
lines that is present at each corner of the arena. If the sensor detects blue line first then
the robot will make a right turn and if it detect orange first then it will make a left turm


**3. BNO055**
The BNO055 provides multiple feature to the robot, it can detect the acceleration of the robot
and also it's dirention as it has a gyro sensor built into it, which helps the robot navigates
the arena as it has access to more information on itself. The BNO055 is positioned on top of
the Raspberry pi Pico wo conserve spaces and for ease of connection.

**4. Fish-eye lens camera**
The fish-eye lens camera provide the robot with a wide view of the arena at the front of the
robot which helps in better obstacle detection and regonition. The camera is connected directly
to the Raspberry Pi 5 as it's the main processor of the image and is positioned at the front to
see the obstacle up ahead of it. The camera is not placed higher in the robot because in doing
so will hinder the abilities of the RPLidar C1 which is at the top.