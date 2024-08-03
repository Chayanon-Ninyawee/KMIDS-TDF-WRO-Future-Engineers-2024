# WRO-2024-Future_Engineer
## Table of contents

- [Introduction](#Introduction)
- [Mobility Management](#Mobility-Management)
- [Power and Sensor Management](#Power&Sensor-Management)
- [Obstacle Management](#Obstacle-Management)
- [Picture(s)](#Pictures)

## Introduction

Welcome to our repository for our project submission to the World Robotic Olympiad(WRO)
in the category of future-engineering compettition. In this project we aim to design an
autonomous vehicle that can navigate around a field with various rules and restriction.

## Mobility-Management

1. Chassis
  - Material: Plastic
  - Description: The chassis for our vehicle is designed with plastic to ease the
    design and prototyping process of the vehicle. The use of 3D printing with plastic 
    helps us keep the weight of the chassis low. The chassis also includes the mounting
    for the various parts of the vehicle, reducing the need for a separate mounting plate.
    Plastic is an excellent choice due to its balance of strength, flexibility, and lightness,
    which are crucial for efficient vehicle movement and stability.
    
2. Wheels and Motor
  - Wheels: four high traction rubber wheels
  - Motor: a single TT motor
  - Description: We decided to use a single DC motor to power the two back wheels
    of our vehicle because of the motor's high power output. We connected it to
    a differential to help the motor distribute its power to the back wheels as
    needed. This setup ensures smooth and efficient power distribution, improving
    the vehicle's traction and maneuverability. The rubber wheels provide the grip
    on the surfaces.

3. Steering Machenism
   - Servo motor: The implementation of a servo motor into the front bridge of our
     vehicle allows for sharp cornering and precise steering control. This mechanism
     is critical for navigating tight spaces and obstacles efficiently.

## Power&Sensor-Management
**Processing Unit**
- **Raspberry pi**
  - **Reason:** A raspberry pi is used to process the video from the camera which decides where
    and what direction the vehicle shouldd head. It's also connected to the Arduino nano and
    issue commands to it. The Raspberry Pi offer us a powerful processing capability suited
    for processing complex images and executing complex algorithm.

**Vision System**
- **Raspberry pi camera**
  - **Reason:** A raspberry pi camera can be intergrate easily into the raspberry pi as
    the raspberry pi have dedicated pins to connect to the camera, couple with the
    camera's good quality make it a good choice for our vehicle, providing clear and
    detailed images to the Raspberry pi

**Distance Sensing**
- **Ultrasonic Sensors**
  - **Placement:** front, back, left and right sides
  - **Reason:** We use four ultrasonic sensor to detect the distance of the vehicle from
    the wall so that the vehicle can stay within an ideal distance from the wall and
    avoiding collision with objects.

**Power Supply**
- **1 lithium ion battry**
  - **Reason:** A single lithium-ion cell provides the necessary voltage to power all
    our electronics and is rechargeable, reducing waste compared to disposable batteries.
    The battery's high energy and rechargeability make it a efficient power source for
    the vehicle.

**Orientation and Motion Sensing**
- **BNO055**
  - **Reason:** The BNO055 sensor measures acceleration and orientation, improving the
    navigational ability of the vehicle and reducing the load on the host processor.
    This sensor combines accelerometer, gyroscope, and magnetometer data to provide
    accurate orientation information, crucial for precise vehicle control.

**Motor Control**
- **Maker Drive**
  - **Reason:** To control the motor, we implement a Maker Drive to regulate the power
    and direction of the motor. This motor driver provides efficient and reliable
    control over the motor's speed and direction, essential for smooth and precise
    vehicle movement.

**Micro Processor**
- **Arduino nano**
  - **Reason:** The arduino nano are connected to various electronics(Maker Drive, Raspberry pi,
    BNO055) and help control all of it for the raspberry pi. The Nano serves as an intermediary,
    handling lower-level tasks and offloading some processing from the Raspberry Pi.



**Wiring Diagram**

![circuit (2)](https://github.com/user-attachments/assets/ee321578-954b-4a7f-b899-d265ed52cab6)

## Obstacle-Management  
**Open Challenge**  
In the open challenge, our vehicle must drive around a field within a time limit 3 minute and
in a total of 3 laps around the field, additionally must not come into contact with the walls 
of the field. The vehicle also must not come into contact with the walls whether it be the 
inner or outer walls.
![open challenge2 drawio (1)](https://github.com/user-attachments/assets/29609437-159f-4914-81d7-570c17d80eff)

**Obstacle Challenge**  
In the obstacle challenge, our vehicle will also needed to complete the requirement as said above,
but the vehicle will also need to avoid additional pillars of green and red that is scatter around
the field. The pillars also have it's individual rules that the vehicle must obey plus a now designated
area for parking the vehicle.
![ObstacleChallenge2 drawio](https://github.com/user-attachments/assets/226d0d06-ef2b-4151-918d-a48813322abe)

## Pictures

**Vehicle Picture**
![IMG_20240728_220240_HDR](https://github.com/user-attachments/assets/e0066bca-ebf5-45b9-8d32-52aec4c62f70)


## Performance video
To be added
