# WRO-2024-Future_Engineer
## Table of contents

- [Introduction](#Introduction)
- [Mobility Management](#Mobility-Management)
- [Power and Sensor Management](#Power&Sensor-Management)
- [Obstacle Management](#Obstacle-Management)
- [Picture(s) and vieos](#Pictures&videos)

## Introduction

Welcome to our repository for our project submission to the World Robotic Olympiad(WRO)
in the category of future-engineering compettition. In this project we aim to design a
autonomous vehicle that can navigate around a field with various rules and restriction.

## Mobility-Management

1. Chassis:
  - Material: Plastic
  - Description: The Chasis for our vehicle is design with plastic to help ease
    the design and prototyping process of the vehicle, couple with the fact that 3D
    printing with platic help us keep the weight of the chasis low. The chassis also
    include the mounting for the various parts of the vehicle, reducing the need for
    a mounting plate in our vehicle.

2. Wheels and Motor
  - Wheels: four high traction rubber wheels
  - Motor: a single TT motor
  - Description: We decided to use a single DC motor to power the two back wheels 
    in our vehicle because of the motor's high power and we connected it to a differential 
    to help the motor distribute it's power to the back wheel as needed.

## Power&Sensor-Management
- We incorporate 4 ultrasonic sensors at each side of the vehicle(front , back,
left and right)
  - Reason: We use 4 ultrasonic sensor to detect the distance of the vehicle from
    the wall so that the vehicle can stay within a certain distance from the wall
    without crashing
- We use Raspberry pi to process the video taken by the camera
  - Reason: A raspberry pi can act as our vehicle's image processing unit that can
    process the video sent by the camera and process it into instruction for the whole
    vehicle. 
- We have a raspberry pi camera at the front of the vehicle
  - Reason: A raspberry pi camera can be intergrate easily into the raspberry pi as
    the raspberry pi have dedicated pins to connect to the camera, couple with the
    camera's good quality make it a good choice for our vehicle.
- We use 1 lithium ion battry to power the whole vehicle
- We have a Maker Drive to distribute the power to the motors in the back wheels
- We have an arduino to manage the control of the motors and the servo at the front bridge
- A BNO055 help us keep track of the acceleration of the vehicle


**Wiring Diagram**

![circuit (1)](https://github.com/user-attachments/assets/c34df00c-26d5-41fc-b7e5-970560ee5f46)


## Obstacle-Management
**Open Challenge**

![open challenge2 drawio](https://github.com/user-attachments/assets/1de2232d-f0d9-41ad-a15a-7fc9c8a928f4)

**Obstacle Challenge**  
(need fixing)

![WRO-obstacle round_02 drawio (1)](https://github.com/user-attachments/assets/f55a3f9a-e7dc-4366-b21c-5738887686cb)

## Pictures&videos

**Vehicle Picture**
![IMG_20240728_220240_HDR](https://github.com/user-attachments/assets/e0066bca-ebf5-45b9-8d32-52aec4c62f70)


## Performance video
tba

