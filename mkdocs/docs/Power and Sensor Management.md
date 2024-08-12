# Power and Sensor Management

## Processing Unit
* **Raspberry Pi**
* **Reason:** A raspberry pi is used to process video from the camera which calculates where and in what direction the vehicle should headed. The Raspberry Pi also connected to the Arduino nano and issued commands to it to lessen the load on the Raspberry Pi itself as it's already processing the algorithm for driving and the images from the Pi camera.

## Vision System
* **Raspberry pi** camera
* **Reason:** The Raspberry Pi camera integrates easily with the Raspberry Pi and provides clear, detailed images, making it a good choice for our vehicle. The camera's high resolution and fast frame rate ensure that the Raspberry Pi receives timely and accurate visual data, which is crucial for making real-time navigation decisions. The camera's compatibility with the Pi also simplifies the hardware setup, enhancing system reliability.

## Distance Sensing
* **gy-530vl53l0x**
* **Placement:** front, back, left and right sides
Reason: We use four gy-530vl53l0x sensors to detect the distance of the vehicle from the wall so that the vehicle can stay within an ideal distance from the wall and avoid colliding with objects that is placed on the field in certain rounds of the competition.

## Power Supply
* **1 lithium-ion battery**
* **Reason:** A single lithium-ion cell provides the necessary voltage to power all our electronics and is rechargeable, reducing waste compared to disposable batteries. The battery's high energy and rechargeability make it an efficient power source for the vehicle's prolonged uses and the power consumption that come with the various electronics.

## Orientation and Motion Sensing
* **BNO055**
* **Reason:** The BNO055 sensor measures acceleration and orientation, improving the vehicle's navigational ability and reducing the load on the host processor. It combines accelerometer, gyroscope, and magnetometer data for accurate orientation information, crucial for precise control. The sensor's ability to provide real-time orientation data helps the vehicle maintain its intended path and respond accurately to changes in direction. This is especially important in dynamic environments where the vehicle needs to make quick adjustments to avoid obstacles and stay on course.

## Motor Control
* **Maker Drive**
**Reason:** To control the motor, we implement a Maker Drive to regulate the power and direction of the motor. This motor driver provides efficient and reliable control over the motor's speed and direction, essential for smooth and precise vehicle movement.

## Micro Processor
* **Arduino nano**
* **Reason:** The Arduino nano is connected to various electronics (Maker Drive, Raspberry pi, BNO055) and helps control all of it for the raspberry pi. The Nano serves as an intermediary, handling lower-level tasks and offloading some processing from the Raspberry Pi.

## Wiring Diagram
![circuit (2)](img\circuit (2).png) 