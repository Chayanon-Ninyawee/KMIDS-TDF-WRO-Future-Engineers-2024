# WRO-2024-Future_Engineer
Team: KMIDS Thermodynamically Favorable (KMIDS TDF)
## Table of contents

1. [Introduction](#Introduction)
2. [Mobility Management](#Mobility_Management)
3. [Power and Sensor Management](#Power_and_Sensor_Management)
4. [Obstacle Management](#Obstacle_Management)
5. [Picture(s)](#Pictures)
6. [Simulator](#Simulator)
7. [Codes](#Code's_Repository)  

## Introduction

Welcome to our repository for our project submission to the World Robotic Olympiad (WRO) 
in the category of future-engineering competition. In this project we aim to design an 
autonomous vehicle that can navigate around a field with various rules and restrictions 
depending on the rounds. Our goal is to create a highly efficient, dependable, and versatile 
vehicle capable of performing complex tasks autonomously.

## Mobility Management

### 1.Chassis
  - Material: Plastic (pla+ from esun)
  - Description: The chassis for our vehicle is designed with plastic to ease the design and
    prototyping process. The use of 3D printing with plastic helps us keep the chassis lightweight.
    It also includes mounting points for the various parts of the vehicle, reducing the need for a
    separate mounting plate. Plastic offers a balance of strength, flexibility, and lightness, crucial
    for efficient vehicle movement and stability. Additionally, the choice of plastic allows for easy
    modifications and quick iterations during the development phase, which is essential for refining
    the design based on testing and performance feedback.
    
### 2.Wheels_and_Motor  
  - Wheels: four high traction rubber wheels
  - Motor: a single TT motor
  - Description: We use a single DC motor to power the two back wheels due to its high-power output.
    It is connected to a differential to help distribute power to the back wheels as needed, ensuring
    smooth and efficient power distribution. The rubber wheels provide a grip on surfaces, preventing
    slippage and improving overall performance. The use of high-traction rubber ensures that the vehicle
    can maneuver effectively on various surfaces encountered during the competition. The differential
    mechanism is crucial for maintaining balance and stability, especially when the vehicle makes sharp turns.

### 3.Steering_Mechanism  
   - Servo motor: The servo motor in the front bridge of our vehicle allows for sharp cornering and
     precise steering control. This mechanism is critical for navigating tight spaces and obstacles
     efficiently. The servo motor's ability to provide fine control over the steering angle enhances
     the vehicle's agility, making it great at avoiding obstacles and navigating through challenging
     courses. The integration of the servo motor into the front bridge design also ensures that the
     steering system is responsive, capable of handling sudden changes in direction without compromising stability.

## 4.3D_Model
The model can be found at: https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024-CAD.git 

**Preview**
![model1](https://github.com/user-attachments/assets/f41a2135-810c-4ccf-9b6c-103ac6d18c37)
![model2](https://github.com/user-attachments/assets/d45dcc8f-3008-4ca5-84d4-a537edc13299)
![model3](https://github.com/user-attachments/assets/24de6c65-e1ef-495b-adf7-759964857b95)
![model4](https://github.com/user-attachments/assets/885f3590-4642-4a53-9ade-b99cad2d2453)

     
## Power_and_Sensor_Management  
### Processing Unit  
- **Raspberry pi**
  - **Reason:** A raspberry pi is used to process the video from the camera which calculate
    where and in what direction the vehicle should headed. The Raspberry Pi also connected
    to the Arduino nano and issue commands to it to lessen the load on the Raspberry Pi itself
    as it's already processing the algorithms for driving and the images from the Pi camera.

### Vision System
- **Raspberry pi camera**
  - **Reason:**  The Raspberry Pi camera integrates easily with the Raspberry Pi and provides
    clear, detailed images, making it a good choice for our vehicle. The camera's high resolution
    and fast frame rate ensure that the Raspberry Pi receives timely and accurate visual data,
    which is crucial for making real-time navigation decisions. The camera's compatibility with
    the Pi also simplifies the hardware setup, enhancing system reliability.
    
### Distance Sensing  
- **gy-530vl53l0x**
  - **Placement:** front, back, left and right sides
  - **Reason:** We use four gy-530vl53l0x sensor to detect the distance of the vehicle from
    the wall so that the vehicle can stay within an ideal distance from the wall and
    avoid colliding with objects that is placed on the field in certain round of
    the competition.

### Power Supply
- **lithium ion batteries**
  - **Reason:** two lithium-ion cells provide the necessary voltage to power all our electronics
    and is rechargeable, reducing waste compared to disposable batteries. The battery's high energy
    and rechargeability make it an efficient power source for the vehicle's prolonged uses and the
    power consumption that come with the various electronics.

### Orientation and Motion Sensing
- **BNO055**
  - **Reason:** The BNO055 sensor measures acceleration and orientation, improving
    the vehicle's navigational ability and reducing the load on the host processor.
    It combines accelerometer, gyroscope, and magnetometer data for accurate orientation
    information, crucial for precise control. The sensor's ability to provide real-time
    orientation data helps the vehicle maintain its intended path and respond accurately
    to changes in direction. This is especially important in dynamic environments where
    the vehicle needs to make quick adjustments to avoid obstacles and stay on course.

### Motor Control
- **Maker Drive**
  - **Reason:** To control the motor, we implement a Maker Drive to regulate the power
    and direction of the motor. This motor driver provides efficient and reliable
    control over the motor's speed and direction, essential for smooth and precise
    vehicle movement.

### Microprocessor
- **Arduino nano**
  - **Reason:** The Arduino nano are connected to various electronics(Maker Drive, Raspberry pi,
    BNO055) and help control all of it for the raspberry pi. The Nano serves as an intermediary,
    handling lower-level tasks and offloading some processing from the Raspberry Pi.



### Wiring Diagram


![circuit (2)](https://github.com/user-attachments/assets/f2e377d0-6cdc-46d0-8369-a7eadb77a23c)


## Obstacle_Management  
### Open Challenge  
In the open challenge, our vehicle must drive around a field within a time limit 3 minute and
in a total of 3 laps around the field, additionally must not come into contact with the walls 
of the field. The vehicle also must not come into contact with the walls whether it be the 
inner or outer walls.


![open challenge2(dark) drawio](https://github.com/user-attachments/assets/a4d2849d-4b81-46ae-acda-06fecf008ce3)


### Obstacle Challenge  
In the obstacle challenge, our vehicle will also needed to complete the requirement as said above,
but the vehicle will also need to avoid additional pillars of green and red that is scatter around
the field. The pillars also have it's individual rules that the vehicle must obey plus a now designated
area for parking the vehicle.  


![ObstacleChallenge2(dark) drawio](https://github.com/user-attachments/assets/2b8b057d-6421-4a8c-8a0d-3db446a31536)


## Pictures
**Vehicle Picture**  

![IMG_20240728_220240_HDR (1)](https://github.com/user-attachments/assets/bea13e5b-34d0-43cb-bb45-c0cf6dfb639b)

## Simulator
Before we implement the vehicle’s code into the actual robot, we need to test if the code can work first.
Our solution to this is to create a simulator to test the code in Godot.

**Code's_Repository**  
The repository for our vehicle's code can be found at:
[https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024-Simulator.git](https://github.com/Chayanon-Ninyawee/KMIDS-TDF-WRO-Future-Engineers-2024-Simulator/tree/main/PythonScripts) 
The repository contains the actual code for the vehicle that's going to be used in the coming competition.
