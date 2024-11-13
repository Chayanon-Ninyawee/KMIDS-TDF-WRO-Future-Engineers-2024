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

## Performance video
our video demonstrating our robot can be found [here](https://youtu.be/9CfIpZZZoUU?si=juIyMaRLtTwPCz_5)

## 3D_Model

Our 3D model are made and design in FreeCAD specificly the [realthunder](https://github.com/realthunder/FreeCAD/releases)
version which is a free open source CAD software that allow a wide range of developer to access the field 
of 3D modeling. Our 3D models can be view in `KMIDS-TDF-WRO2024-FreeCAD`

## Chassis
  - Material: Plastic (pla+ from esun)
  - Description: The chassis for our robot is made with 3D printer to facilitate a chassis that
    is durable and flexible to endure the stress that is put on it in the competition. The 3D
    printer also allowed us to prototype various parts in quick succession, creating an effective
    workflow.

## Specification
  |Section|Component|Description|Image|
  |-------|---------|-----------|-----|
  |Wheels|[69916 TAMIYA Robot Sports Tire](https://shopee.co.th/69916-TAMIYA-Robot-Sports-Tire-Set-(56mm-Dia.-Clear-Wheels)-i.17469725.6833009480) |small rubber tire that is made to be in a miniature robot. The size of the wheels provide our robot with a more compact design which helps in parking|![Tamiya Tires](https://github.com/user-attachments/assets/e0556fc9-7187-458d-8897-592a2f78318d)|
  |Motor|[3V 1350RPM DC Micro Metal Gearmotor](https://th.cytron.io/p-3v-1350rpm-dc-micro-metal-gearmotor?gclid=CjwKCAjw9eO3BhBNEiwAoc0-jZ36NARTI2F-goyzNqGCDIobAW_VJRe-QqXPy0r8zaLhQzDTmzUP6hoCYvEQAvD_BwE)|This motor offer a powerful rotaion in a small sizes that we see as beneficial in our robot as it allow us to make a more compact design for the robot which make for narrower turns.|![DC gear motor](https://github.com/user-attachments/assets/3ba3144c-acc7-4967-8b88-a99e5bdde6b7)|
  |Servo|[MG90S Metal Gear Micro Servo](https://th.cytron.io/c-dc-motor/p-mg90s-metal-gear-micro-servo?gclid=Cj0KCQjwyL24BhCtARIsALo0fSAMEsRi6i9XiLhB3JzwqaTsE8m8xMBNWKnkg4yPYmgYKS4qhMOHMRgaAjZUEALw_wcB)|this servo possess a lot of gear which helps in refining our control and the precision of the robot which is crutial when making turn at an extreme angle to avoid obstacles in quick succession.|![MG90S](https://github.com/user-attachments/assets/3b76b29f-294a-4917-ac4c-38e76179eb99)|
  |Processing Unit|[Raspberry pi V5](https://th.cytron.io/c-carrier-board-for-rpi-cm/p-raspberry-pi-5?gclid=Cj0KCQjwyL24BhCtARIsALo0fSCA1cSwSxPTeWjvmnfoP2jWKKkocSS7wGCum3iJqgFwGyWFi0PRdwQaAibgEALw_wcB)|used to process the video from the camera which calculate where and in what direction the robot should headed.|![Pi v5](https://github.com/user-attachments/assets/7c810df0-46fb-49ca-8058-c27ba22f9bd0)|
  |Processing Unit|[Raspberry pi pico](https://th.cytron.io/p-raspberry-pi-pico?srsltid=AfmBOop-tQfKoMxJU1gJo2nNFrc_FSLUJof0p1Rg2VGSW7uQmTslqJTn)|This helps in commanding other components that Raspberry pi v5 does not|![Pi pico](https://github.com/user-attachments/assets/3985ca39-caf8-4d18-b7ce-838a3c9516a3)|
  |Vision system|[Raspberry pi fisheye len camera](https://th.cytron.io/p-fish-eye-lense-raspberry-pi-5mp-ir-camera?r=1&language=en-gb&gad_source=1&gclid=Cj0KCQjwyL24BhCtARIsALo0fSAs3XDrwvudJq3gCRJTOBm2JJ4lhCwdpE56E3P_x5ZEH4nZM4p4sKkaArvVEALw_wcB)|Raspberry Pi camera integrates easily with the Raspberry Pi and provides clear image in a wide area. Making the algorithm more effective as it's able to recieve a wider view of the field.|![Camera](https://github.com/user-attachments/assets/c6f92078-652c-40f3-96e4-62c2740defa6)|
  |Distance Sensing|[RPLIDAR C1](https://shopee.co.th/Kiki-RPLIDAR-C1-%E0%B9%82%E0%B8%A1%E0%B8%94%E0%B8%B9%E0%B8%A5%E0%B9%80%E0%B8%8B%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%95%E0%B8%A3%E0%B8%A7%E0%B8%88%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B8%A3%E0%B8%B1%E0%B8%87%E0%B8%AA%E0%B8%B5%E0%B8%A2%E0%B8%B9%E0%B8%A7%E0%B8%B5-2D-%E0%B8%AB%E0%B8%A1%E0%B8%B8%E0%B8%99%E0%B9%84%E0%B8%94%E0%B9%89-360-%E0%B8%AD%E0%B8%87%E0%B8%A8%E0%B8%B2-%E0%B8%AA%E0%B9%8D%E0%B8%B2%E0%B8%AB%E0%B8%A3%E0%B8%B1%E0%B8%9A%E0%B8%AB%E0%B8%B8%E0%B9%88%E0%B8%99%E0%B8%A2%E0%B8%99%E0%B8%95%E0%B9%8C-i.409507050.25664846291)|Detect objects in a 360 angle and within the range of 12 meters. It also takes 5,000 samples per second, providing clear and accurate readings.|![lidar](https://github.com/user-attachments/assets/97e8bd01-c672-4faf-983c-e6ed3697aad8)|
  |Color Sensing|[Light Sensor TC01](https://shopee.co.th/%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99-Light-Sensor-TC01-(%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99)-JST2.0-%E0%B8%9E%E0%B8%A3%E0%B9%89%E0%B8%AD%E0%B8%A1%E0%B8%AA%E0%B8%B2%E0%B8%A2-JST-3-pin-Phototransistor-%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%95%E0%B8%A3%E0%B8%A7%E0%B8%88%E0%B8%88%E0%B8%B1%E0%B8%9A%E0%B9%80%E0%B8%AA%E0%B9%89%E0%B8%99-i.72015392.16106103845)|Detect color of objects|![Light Sensor](https://github.com/user-attachments/assets/3f4d103f-4d66-4223-8793-1fcffac3a8db)|
  |Power Supply|Lithium ion batteries|provides power for the robot|![lithium ion batteries](https://github.com/user-attachments/assets/ee19daea-2eb3-4729-9f99-aff8291d4801)|
  |Orientation and motion Sensing|[BNO055](https://shopee.co.th/BNO055-%E0%B9%82%E0%B8%A1%E0%B8%94%E0%B8%B9%E0%B8%A5%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C-9-DOF-%E0%B8%A3%E0%B8%B8%E0%B9%88%E0%B8%99-Halley-V1-%E0%B8%AD%E0%B9%88%E0%B8%B2%E0%B8%99%E0%B8%84%E0%B9%88%E0%B8%B2%E0%B8%A1%E0%B8%B8%E0%B8%A1-IMU-MPU-Angle-Massmore-Product-i.5641091.24661859112)|Measures acceleration and orientation comes with accelerometer, gyroscope, magnetometer|![BNO055](https://github.com/user-attachments/assets/42a6e7ae-de5f-4537-9d87-f5e2a32081af)|
  |Motor Control|[L9110S H-bridge Stepper Motor Dual DC Driver Controller Board](https://shopee.co.th/product/5401692/1540697025?gads_t_sig=VTJGc2RHVmtYMTlxTFVSVVRrdENkVjhKejlrTjhjZ0djRXFyYU5xR2swSUVHNmtGUDVTWDdxSzRyUWVFZGYwUDdxVmIrRUxDN09xZ05ETXdTQlpXNEd1UkszZ3BHN3lEbWpsMDJmSFRyMEJ6ZkcyZldkVmY0NXR0NTloMUEvTkM&gad_source=1&gclid=Cj0KCQjw05i4BhDiARIsAB_2wfBuI_zh93yA1Pe3dZ3mnCmLtWkGAH8RJ_enMkRA6Dci5gDbjywpG8IaAu1tEALw_wcB)|Control and regulate the 3V 1350RPM DC Micro Metal Gearmotor|![H-bridge](https://github.com/user-attachments/assets/730f70fb-50dc-4761-b439-ddc92e103136)|

## Wiring Diagram

![circuit](https://github.com/user-attachments/assets/7a223edd-bf7f-4c09-91c3-a9a39c628697)


## Obstacle_Management  
### Open Challenge  
In the open challenge, our robot must drive around a field within a time limit 3 minute and
in a total of 3 laps around the field, additionally must not come into contact with the walls 
of the field. The robot also must not come into contact with the walls whether it be the 
inner or outer walls.

![Open chart](https://github.com/user-attachments/assets/ddbccffe-5dab-4fce-bd86-55afe057d02e)

### Obstacle Challenge  
In the obstacle challenge, our robot will also needed to complete the requirement as said above,
but the robot will also need to avoid additional pillars of green and red that is scatter around
the field. The pillars also have it's individual rules that the robot must obey plus a now designated
area for parking the robot.

![Obstacle chart](https://github.com/user-attachments/assets/fd552782-8ea8-4165-8fa0-5567a0384821)


## Design Journey
When designing the robot, the first thing that we ruminated on is how the robot will park. This is a very
important step to consider because we can choose to have it park back first into the wall or have it do a
paralell park. The secon thing that we consider is the turning angle of the robot: how to optimise it so 
that it have the maximum turn angle to help avoid obstacles. Then comes the process of designing the chassis
and where to put various electronics that we want on it, a major problem in this step is the limitation of
our RPLIDAR C1 that can only be at a certain heign else it won't detect the wall. The last and most annoying
part for us is the prototyping phase where we need to keep printing and readjust the model, because the actual
3D parts can deviate from the original design.
