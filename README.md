# WRO-2024-Future_Engineer
Team: KMIDS Thermodynamically Favorable (KMIDS TDF)

In this project we aim to design an autonomous vehicle that can navigate around a field with 
various rules and restrictions depending on the rounds. Our goal is to create a robot that can
achieve the objective sets by the compettition.

## Mobility Management

### 1.Chassis
  - Material: Plastic (pla+ from esun)
  - Description: The chassis for our vehicle is made with 3D printer to facilitate a chasis that
    is durable and flexible to endure the stress that is put on it in the compettition. The 3D
    printer also allowed us to prototype various parts in quick succession, creating an effective
    workflow.
    
### 2.Wheels_and_Motor  
  - Wheels: [69916 TAMIYA Robot Sports Tire](https://shopee.co.th/69916-TAMIYA-Robot-Sports-Tire-Set-(56mm-Dia.-Clear-Wheels)-i.17469725.6833009480)
      - 69916 TAMIYA Robot Sports Tire is a small rubber tire that is made to be in a miniature robot.
        The size of the wheels provide our vehicle with a more compact design which helps in parking
        the robot in the obstacle challenge round, in which we have to park the vehicle in a certain
        spot that is quite limited.
        
  - Motor: [6V 440RPM DC Micro Metal Gearmotor](https://th.cytron.io/p-6v-440rpm-dc-micro-metal-gearmotor?srsltid=AfmBOooc6VFJJZNSbLRc4bHKPMXPgumGJDo1YQ_qqBssI0HRBYQ6wNjJ)
      - This motor offer a powerful rotaion in a small sizes that we see as beneficial in our robot
        as it allow us to make a more compact design for the robot which make for narrower turns.
        The speed advantages that the motor provide us will also be good for us when being taken
        into consideration for score.

### 3.Steering_Mechanism  
   - [MG90S Metal Gear Micro Servo](https://th.cytron.io/c-dc-motor/p-mg90s-metal-gear-micro-servo?gclid=Cj0KCQjwyL24BhCtARIsALo0fSAMEsRi6i9XiLhB3JzwqaTsE8m8xMBNWKnkg4yPYmgYKS4qhMOHMRgaAjZUEALw_wcB)
       - We decide to use this servo because it posses a lot of gear which helps in refining our control
         and the precision of the robot which is crutial when making turn at an extreme angle to avoid
         obstacles in quick succession. We especcially selected this servo in place of a normal servo
         because of it's precision when compare to common servo that are seen commonly.

## 4.3D_Model

Our 3D model are made and design in FreeCAD specificly the [realthunder](https://github.com/realthunder/FreeCAD/releases)
version which is a free open source CAD software that allow a wide range of developer to access the field 
of 3D modeling. FreeCAD also provide a more noticable look into Github changes as it's file are vieable 
without downloading.

### Picture


## Power_and_Sensor_Management  
### Processing Unit  
- **[Raspberry pi V5](https://th.cytron.io/c-carrier-board-for-rpi-cm/p-raspberry-pi-5?gclid=Cj0KCQjwyL24BhCtARIsALo0fSCA1cSwSxPTeWjvmnfoP2jWKKkocSS7wGCum3iJqgFwGyWFi0PRdwQaAibgEALw_wcB)**
  - **Reason:** A raspberry pi is used to process the video from the camera which calculate
    where and in what direction the vehicle should headed. The Raspberry Pi also connected
    to the Arduino nano and issue commands to it to lessen the load on the Raspberry Pi itself
    as it's already processing the algorithms for driving and the images from the Pi camera.

### Vision System
- **[Raspberry pi fisheye len camera](https://th.cytron.io/p-fish-eye-lense-raspberry-pi-5mp-ir-camera?r=1&language=en-gb&gad_source=1&gclid=Cj0KCQjwyL24BhCtARIsALo0fSAs3XDrwvudJq3gCRJTOBm2JJ4lhCwdpE56E3P_x5ZEH4nZM4p4sKkaArvVEALw_wcB)**
  - **Reason:**  The Raspberry Pi camera integrates easily with the Raspberry Pi and provides
    clear image in a wide area. Making the algorithm more effective as it's able to recieve
    a wider view of the field.
    
### Distance Sensing  
- **[RPLIDAR C1](https://shopee.co.th/Kiki-RPLIDAR-C1-%E0%B9%82%E0%B8%A1%E0%B8%94%E0%B8%B9%E0%B8%A5%E0%B9%80%E0%B8%8B%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%95%E0%B8%A3%E0%B8%A7%E0%B8%88%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B8%A3%E0%B8%B1%E0%B8%87%E0%B8%AA%E0%B8%B5%E0%B8%A2%E0%B8%B9%E0%B8%A7%E0%B8%B5-2D-%E0%B8%AB%E0%B8%A1%E0%B8%B8%E0%B8%99%E0%B9%84%E0%B8%94%E0%B9%89-360-%E0%B8%AD%E0%B8%87%E0%B8%A8%E0%B8%B2-%E0%B8%AA%E0%B9%8D%E0%B8%B2%E0%B8%AB%E0%B8%A3%E0%B8%B1%E0%B8%9A%E0%B8%AB%E0%B8%B8%E0%B9%88%E0%B8%99%E0%B8%A2%E0%B8%99%E0%B8%95%E0%B9%8C-i.409507050.25664846291)**
  - We selected teh RPLIDAR C1 as our vision system because if it's ability to detect object
    in a 360 degree and take up to 5,000 samples per seconds, combine with it's range of 12
    meter this sensor will help us improve the navigational abilities of our autonomous vehicle.
    We see the need to use this sensor from our previous test that a normal infared sensors can
    malfunction when placed in environments that have high amount of infared lights (outdoor,
    old lightbulb).

### Color sensing
- **[Light Sensor TC01](https://shopee.co.th/%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99-Light-Sensor-TC01-(%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99)-JST2.0-%E0%B8%9E%E0%B8%A3%E0%B9%89%E0%B8%AD%E0%B8%A1%E0%B8%AA%E0%B8%B2%E0%B8%A2-JST-3-pin-Phototransistor-%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%95%E0%B8%A3%E0%B8%A7%E0%B8%88%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99-i.72015392.16106103845)**
    - We implement these light sensors to have a better tracking of the obstacle cource and
      the direction that the vehicle is moving in. The main purpose of the light sensors is
      to detected the orange and blue line that indicate the direction of turning. We decide
      to use two of these sensors obecause it give us better tracking of which color line come
      first.

### Power Supply
- **lithium ion batteries**
  - **Reason:** two lithium-ion cells provide the necessary voltage to power all our electronics
    and is rechargeable, reducing waste compared to disposable batteries. The battery's high energy
    and rechargeability make it an efficient power source for the vehicle's prolonged uses and the
    power consumption that come with the various electronics.

### Orientation and Motion Sensing
- **[BNO055](https://shopee.co.th/BNO055-%E0%B9%82%E0%B8%A1%E0%B8%94%E0%B8%B9%E0%B8%A5%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C-9-DOF-%E0%B8%A3%E0%B8%B8%E0%B9%88%E0%B8%99-Halley-V1-%E0%B8%AD%E0%B9%88%E0%B8%B2%E0%B8%99%E0%B8%84%E0%B9%88%E0%B8%B2%E0%B8%A1%E0%B8%B8%E0%B8%A1-IMU-MPU-Angle-Massmore-Product-i.5641091.24661859112)**
  - **Reason:** The BNO055 sensor measures acceleration and orientation, improving
    the vehicle's navigational ability and reducing the load on the host processor.
    It combines accelerometer, gyroscope, and magnetometer data for accurate orientation
    information, crucial for precise control. The sensor's ability to provide real-time
    orientation data helps the vehicle maintain its intended path and respond accurately
    to changes in direction. This is especially important in dynamic environments where
    the vehicle needs to make quick adjustments to avoid obstacles and stay on course.

### Motor Control
- **[L9110S H-bridge Stepper Motor Dual DC Driver Controller Board](https://shopee.co.th/product/5401692/1540697025?gads_t_sig=VTJGc2RHVmtYMTlxTFVSVVRrdENkVjhKejlrTjhjZ0djRXFyYU5xR2swSUVHNmtGUDVTWDdxSzRyUWVFZGYwUDdxVmIrRUxDN09xZ05ETXdTQlpXNEd1UkszZ3BHN3lEbWpsMDJmSFRyMEJ6ZkcyZldkVmY0NXR0NTloMUEvTkM&gad_source=1&gclid=Cj0KCQjw05i4BhDiARIsAB_2wfBuI_zh93yA1Pe3dZ3mnCmLtWkGAH8RJ_enMkRA6Dci5gDbjywpG8IaAu1tEALw_wcB)**
  - **Reason:** To control regulate the motor, we implement a L9110S H-bridge Stepper
    Motor to control the 6V 440RPM DC Micro Metal Gearmotor's power.


### Wiring Diagram

![circuit (1)](https://github.com/user-attachments/assets/5d74f425-f17c-4a31-8923-1de4361dd4a8)




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

tba

