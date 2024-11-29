# How to Setup RaspberryPi5
- Insert a microSD card, which should contain 8GB or larger into computer.
- Download, install and run [Raspberry Pi Imager](https://www.raspberrypi.com/software/) that is appropriate for your operating system.
- Click the choose device button and a list of Pi boards appears.
 
<img src="https://cdn.mos.cms.futurecdn.net/SPUHaV6g8Qxc2yBNd6qFW8-970-80.png" width="400" />

- Choose Raspberry Pi 5 in the Pi board. 
  
<img src="https://robu.in/wp-content/uploads/2024/01/o7.jpg" width="500" />

- Click the Choose OS button and menu will appear. 

<img src="https://cdn.mos.cms.futurecdn.net/6sCCwQaNoV3EXVUdrDGweh-970-80.png" width="400" />

- Choose the top choice, Raspberry Pi OS (64-bit), for Pi 5. If there are Raspberry Pi OS on top choice, click that and choose the latest version that works with your board.

<img src="https://robu.in/wp-content/uploads/2024/01/o6.jpg" width="500" />

- Click Choose Storage and select the card from the menu.

<img src="https://robu.in/wp-content/uploads/2024/01/o8.jpg" width="500" />

- Click Next 

<img src="https://cdn.mos.cms.futurecdn.net/cQHK7tWkKGRENVuMkR5Gkg-970-80.png.webp" width="400" />

- Clear and Click Edit Settings 

<img src="https://robu.in/wp-content/uploads/2024/01/o9.jpg" width="400" />

- Fill in all blank on the General tab: hostname, username / password, wireless LAN (If you plan to use Wi-Fi, and locale settings).

<img src="https://cdn.mos.cms.futurecdn.net/Et4hHahUd3dN3nufsLKqFN-970-80.png.webp" width="400" />

- Go to Serivec Tab, Click Enable SSH button and Select "Use password authentication."  Then click Save.

<img src="https://cdn.mos.cms.futurecdn.net/FQPA4pWp9qswNM8feDE4ye-970-80.png.webp" width="400" />

- Click Yes to apply OS customization settings. !

<img src="https://cdn.mos.cms.futurecdn.net/z3SSm8nAART9rhkdxr3jvk-970-80.png.webp" width="400" />

- Click Yes to confirm that you want to write to your microSD card. !

<img src="https://cdn.mos.cms.futurecdn.net/4WM6JqmAUGPpmXJxqMzfC6-970-80.png.webp" width="400" />

- It will take a few minutes to download the OS and write it to the card. 

<img src="https://cdn.mos.cms.futurecdn.net/upTCsdvyixsdfuyhtyKJdQ-970-80.png.webp" width="400" />

- Once the process is complete, insert the microSD card into your Raspberry Pi, power it on, and give it a few seconds to connect to the network. Then, you can try logging in via SSH, assuming both the Raspberry Pi and your computer are connected to the same Wi-Fi network.

# How to install CMake
``` 
sudo apt install -y cmake
```

# How to Setup OpenCV
## Dependencies
OpenCV needs other software libraries to work, and these must be installed first. Some of them come with the Raspberry Pi 64-bit operating system, and others might already be installed. To be safe, here's the full list. Only the latest packages are installed according to the procedure.
```
# check for updates
sudo apt-get update
sudo apt-get upgrade
# dependencies
sudo apt-get install build-essential cmake git unzip pkg-config
sudo apt-get install libjpeg-dev libpng-dev
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install libgtk2.0-dev libcanberra-gtk* libgtk-3-dev
sudo apt-get install libgstreamer1.0-dev gstreamer1.0-gtk3
sudo apt-get install libgstreamer-plugins-base1.0-dev gstreamer1.0-gl
sudo apt-get install libxvidcore-dev libx264-dev
sudo apt-get install python3-dev python3-numpy python3-pip
sudo apt-get install libtbb2 libtbb-dev libdc1394-22-dev
sudo apt-get install libv4l-dev v4l-utils
sudo apt-get install libopenblas-dev libatlas-base-dev libblas-dev
sudo apt-get install liblapack-dev gfortran libhdf5-dev
sudo apt-get install libprotobuf-dev libgoogle-glog-dev libgflags-dev
sudo apt-get install protobuf-compiler
```
## Download OpenCV
Once all the third-party software is installed, you can download OpenCV itself. You'll need two packages: the basic version and the additional contributions. After downloading, simply unzip the files. Be careful with line wrapping in the command boxes. The two commands begin with ```wget``` and end with .```zip.```
```
# check your memory first
free -m
# you need at least a total of 5.8 GB!
# if not, enlarge your swap space as explained earlier
# download the latest version
cd ~
git clone --depth=1 https://github.com/opencv/opencv.git
git clone --depth=1 https://github.com/opencv/opencv_contrib.git
```
## Build Make
Before begin with the actual build of the library. You have to make a directory where all the build files can be located.
```
cd ~/opencv
mkdir build
cd build
```
In this step, you instruct CMake on what , where , and how to make OpenCV on your Raspberry Pi. There are several flags to consider, most of which will be familiar to you. One important line you’ll notice is -D WITH_QT=OFF, which disables Qt5 support. If you want to use Qt5 for the GUI, change this to -D WITH_QT=ON. We also save space by excluding any (Python) examples or tests.

Note that there should be spaces before the -D flags, not tabs. Also, the two dots at the end are no typo. It tells CMake where to find its CMakeLists.txt file (the main configuration file); one directory up.
```
cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
-D ENABLE_NEON=ON \
-D WITH_OPENMP=ON \
-D WITH_OPENCL=OFF \
-D BUILD_TIFF=ON \
-D WITH_FFMPEG=ON \
-D WITH_TBB=ON \
-D BUILD_TBB=ON \
-D WITH_GSTREAMER=ON \
-D BUILD_TESTS=OFF \
-D WITH_EIGEN=OFF \
-D WITH_V4L=ON \
-D WITH_LIBV4L=ON \
-D WITH_VTK=OFF \
-D WITH_QT=OFF \
-D WITH_PROTOBUF=ON \
-D OPENCV_ENABLE_NONFREE=ON \
-D INSTALL_C_EXAMPLES=OFF \
-D INSTALL_PYTHON_EXAMPLES=OFF \
-D OPENCV_FORCE_LIBATOMIC_COMPILER_CHECK=1 \
-D PYTHON3_PACKAGES_PATH=/usr/lib/python3/dist-packages \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D BUILD_EXAMPLES=OFF ..
```
CMake comes with a report, showing image below. ![CMake](https://qengineering.eu/images/CMakeSucces64.webp)
## Make
With all compilation directives in place, you can start the build with the following command. This will take about minutes.
```
make -j4
```
Hope that it will show the piture below: ![](https://qengineering.eu/images/CMakeSucces64_4_5_0.webp)
Install all the generated packages to the database of your system with the next commands.
```
sudo make install
sudo ldconfig
# cleaning (frees 300 KB)
 make clean
sudo apt-get update
```
## Check
Use the following C++ code to check the OpenCV
```
#include <opencv2/opencv.hpp>

int main(void)
{
    std::cout << "OpenCV version : " << cv::CV_VERSION << endl;
    std::cout << "Major version : " << cv::CV_MAJOR_VERSION << endl;
    std::cout << "Minor version : " << cv::CV_MINOR_VERSION << endl;
    std::cout << "Subminor version : " << cv::CV_SUBMINOR_VERSION << endl;
    std::cout << cv::getBuildInformation() << std::endl;
}
```

# How to connect to Raspberry pi 5 via ethernet cable (Open DHCP Server on the Ethernet port in Linux)
- ### Select interface with:
  ```
  sudo nano /etc/default/isc-dhcp-server
  ---
  INTERFACESv4="eno1"
  INTERFACESv6=""

  sudo nano /etc/dhcp/dhcpd.conf
  ---
  subnet 192.168.39.0 netmask 255.255.255.0 {
  range 192.168.39.10 192.168.39.50;
  option subnet-mask 255.255.255.0;
  option routers 192.168.39.1;
  }
  ```

- ### Add new Ethernet Connection
  make the ipv4 address the same as above (192.168.39.1)
- ### How to run:
  ```
  sudo systemctl start isc-dhcp-server.service
  ```
- ### How to check:
  ```
  sudo systemctl status isc-dhcp-server.service
  ```
- ### If error:
  ```
  journalctl _PID=<PID Here>
  ```
- ### Don't forget to make sure the interface is up:
  ```
  ip addr show eno1
  ```
- ### How to autostart:
  ```
  sudo systemctl enable isc-dhcp-server.service
  ```
- ### How to stop autostart:
  ```
  sudo systemctl disable isc-dhcp-server.service
  ```


# ObstacleChallenge Documentation

## Overview

The `ObstacleChallenge` class provides functionality to navigate through obstacle courses using LIDAR and camera data. It incorporates PID controllers for precise control of steering and wall-following distances, state machines for navigation logic, and traffic light detection for enhanced decision-making.

---

## Enums

### `State`
Defines the various states the robot can be in during the obstacle challenge.

- **Values**:
  - `STOP`
  - `NORMAL`
  - `SLOW_BEFORE_TURN`
  - `WAITING_FOR_TURN`
  - `TURNING`
  - `UTURNING_1`
  - `UTURNING_2`
  - `FIND_PARKING_ZONE`
  - `PARKING_1`
  - `PARKING_2`

---

### `TrafficLightRingPosition`
Specifies the ring position of detected traffic lights.

- **Values**:
  - `OUTER`
  - `INNER`

---

### `TrafficLightPosition`
Defines the relative positions of traffic lights.

- **Values**:
  - `BLUE`
  - `MID`
  - `ORANGE`
  - `NO_POSITION`

---

## Structures

### `TrafficLightSearchKey`
Defines a search key for identifying traffic lights.

- **Fields**:
  - `lightPosition`: A `TrafficLightPosition` specifying the traffic light's position.
  - `direction`: A `Direction` enum representing the direction.

#### Operators
- `operator<`: Compares two `TrafficLightSearchKey` objects for ordering.
- `operator==`: Checks equality of two `TrafficLightSearchKey` objects.

#### Hash Function
A custom hash function is defined for use with `std::unordered_set`:
```cpp
namespace std {
    template <>
    struct hash<TrafficLightSearchKey> {
        size_t operator()(const TrafficLightSearchKey& key) const {
            size_t h1 = std::hash<int>{}(static_cast<int>(key.lightPosition));
            size_t h2 = std::hash<int>{}(static_cast<int>(key.direction));
            return h1 ^ (h2 << 1);
        }
    };
}
```

---

## Class: `ObstacleChallenge`

### Constructor
```cpp
ObstacleChallenge(int lidarScale, cv::Point lidarCenter);
```
Initializes an `ObstacleChallenge` object.

- **Parameters**:
  - `lidarScale`: Scale factor for LIDAR data.
  - `lidarCenter`: Center point of the LIDAR map.

---

### Public Methods

#### **`void update`**
```cpp
void update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent);
```
Updates the robot's state and controls motor and steering based on sensor inputs.

- **Parameters**:
  - `lidarBinaryImage`: Binary image from the LIDAR data.
  - `cameraImage`: Image captured from the camera.
  - `gyroYaw`: Current yaw angle from the gyroscope.
  - `motorPercent`: Motor speed percentage (output parameter).
  - `steeringPercent`: Steering angle percentage (output parameter).

---

### Private Members

#### **PID Controllers**
- `steeringPID`: Controls steering with parameters `Kp = 0.026f`, `Ki = 0.0f`, `Kd = 0.0008f`.
- `wallDistancePID`: Controls wall-following distance with parameters `Kp = 200.0f`, `Ki = 0.0010f`, `Kd = 0.0f`.

#### **Thresholds and Biases**
- `MAX_HEADING_ERROR`: Maximum allowable heading error (`40.0` degrees).
- `MIN_HEADING_ERROR`: Minimum allowable heading error (`-40.0` degrees).
- `FRONT_WALL_DISTANCE_*`: Various thresholds for wall distances, used to make decisions during navigation.
- `wallDistanceBias`: Bias for wall distance control (`0.0f` by default).

#### **State and Cooldown Timers**
- `state`: Current state of the robot (`State::NORMAL` by default).
- `TURN_COOLDOWN`: Cooldown time between turns (`1.1f` seconds).
- `FIND_PARKING_COOLDOWN`: Cooldown time for parking detection (`1.1f` seconds).
- `TRAFFIC_COOLDOWN`: Cooldown time for traffic light processing (`0.8f` seconds).

#### **Traffic Light Handling**
- `trafficLightMap`: Maps `TrafficLightSearchKey` to ring position and color.
- `trafficLightOrderQueue`: Maintains the insertion order of traffic light keys.
- `trafficLightKeySet`: Ensures fast duplicate detection of traffic lights.

#### **Miscellaneous**
- `robotDirection`: Current direction of the robot (`NORTH` by default).
- `turnDirection`: Current turn direction (`UNKNOWN` by default).
- `numberofTurn`: Number of turns completed.

---

## Example Usage
```cpp
#include "ObstacleChallenge.h"
#include <opencv2/opencv.hpp>

int main() {
    cv::Point lidarCenter(100, 100);
    int lidarScale = 10;

    ObstacleChallenge obstacleChallenge(lidarScale, lidarCenter);

    cv::Mat lidarBinaryImage; // Load or initialize LIDAR image
    cv::Mat cameraImage;      // Load or initialize camera image
    float gyroYaw = 0.0f;
    float motorPercent = 0.0f;
    float steeringPercent = 0.0f;

    obstacleChallenge.update(lidarBinaryImage, cameraImage, gyroYaw, motorPercent, steeringPercent);

    // Use motorPercent and steeringPercent as needed
    return 0;
}
```

---

## Licensing

This header file should be used in compliance with the licensing terms of its respective project and dependencies, including OpenCV and any LIDAR processing libraries.


# OpenChallenge Documentation

## Overview

The `OpenChallenge` class provides the logic for navigating an open space using LIDAR data. It employs PID controllers to manage steering and wall distance, and a state machine to handle transitions between different navigation states.

---

## Enums

### `State`
Defines the possible states of the robot during the open challenge.

- **Values**:
  - `STOP`: The robot is stopped.
  - `NORMAL`: The robot is moving forward under normal conditions.
  - `TURNING`: The robot is in a turning state.

---

## Class: `OpenChallenge`

### Constructor
```cpp
OpenChallenge(int lidarScale, cv::Point lidarCenter);
```
Initializes an `OpenChallenge` object.

- **Parameters**:
  - `lidarScale`: Scale factor for the LIDAR data.
  - `lidarCenter`: The center point of the LIDAR map.

---

### Public Methods

#### **`void update`**
```cpp
void update(const cv::Mat& lidarBinaryImage, float gyroYaw, float& motorPercent, float& steeringPercent);
```
Updates the robot's motor and steering outputs based on the LIDAR image and gyro data.

- **Parameters**:
  - `lidarBinaryImage`: Binary image representing the LIDAR data.
  - `gyroYaw`: Current yaw angle from the gyroscope.
  - `motorPercent`: Output parameter for motor speed as a percentage (-1.0 to 1.0).
  - `steeringPercent`: Output parameter for steering angle as a percentage (-1.0 to 1.0).

---

### Private Members

#### **PID Controllers**
- `steeringPID`: Controls steering with parameters:
  - `Kp = 0.017f`
  - `Ki = 0.0f`
  - `Kd = 0.0019f`
- `wallDistancePID`: Controls wall-following with parameters:
  - `Kp = 50.0f`
  - `Ki = 0.0f`
  - `Kd = 0.0030f`

#### **Thresholds and Biases**
- `MAX_HEADING_ERROR`: Maximum allowable heading error (`25.0` degrees).
- `MIN_HEADING_ERROR`: Minimum allowable heading error (`-25.0` degrees).
- `FRONT_WALL_DISTANCE_STOP_THRESHOLD`: Distance at which the robot stops moving forward (`2.200` meters).
- `FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD`: Distance at which the robot slows down (`1.000` meters).
- `FRONT_WALL_DISTANCE_TURN_THRESHOLD`: Distance at which the robot initiates a turn (`0.690` meters).
- `OUTER_WALL_DISTANCE`: Distance to maintain from the outer wall (`0.350` meters).
- `MAX_HEADING_ERROR_BEFORE_EXIT_TURNING`: Maximum heading error allowed before exiting a turning state (`10.0` degrees).
- `wallDistanceBias`: Wall distance bias (`0.000` by default, where negative is left bias and positive is right bias).

#### **State and Cooldown Timers**
- `state`: Current state of the robot (`State::NORMAL` by default).
- `lastTurnTime`: Time of the last turn made (`-1.0` seconds by default).
- `TURN_COOLDOWN`: Cooldown time between consecutive turns (`1.0` seconds).
- `STOP_COOLDOWN`: Cooldown time before stopping after a turn (`0.2` seconds).

#### **Navigation Parameters**
- `lidarScale`: Scale of the LIDAR data.
- `lidarCenter`: Center point of the LIDAR map.
- `robotDirection`: Current direction of the robot (`NORTH` by default).
- `turnDirection`: Current turn direction (`UNKNOWN` by default).
- `numberofTurn`: Number of turns completed.

---

## Example Usage

```cpp
#include "OpenChallenge.h"
#include <opencv2/opencv.hpp>

int main() {
    cv::Point lidarCenter(100, 100);
    int lidarScale = 10;

    OpenChallenge openChallenge(lidarScale, lidarCenter);

    cv::Mat lidarBinaryImage; // Initialize or load LIDAR binary image
    float gyroYaw = 0.0f;
    float motorPercent = 0.0f;
    float steeringPercent = 0.0f;

    openChallenge.update(lidarBinaryImage, gyroYaw, motorPercent, steeringPercent);

    // Use motorPercent and steeringPercent for robot control
    return 0;
}
```

---

## Licensing

This file should be used in compliance with the licensing terms of its respective project and any dependencies such as OpenCV.

# BNO055 Struct Documentation

## Overview

This header file defines structures for handling BNO055 sensor data outputs in precision floating-point format. The structures encapsulate data for accelerometer readings and Euler angles.

---

## Structs

### `bno055_accel_float_t`
A structure representing accelerometer output data in floating-point precision.

#### Fields
- `x`: Accelerometer data along the X-axis.
- `y`: Accelerometer data along the Y-axis.
- `z`: Accelerometer data along the Z-axis.

#### Example
```cpp
bno055_accel_float_t accelData;
accelData.x = 1.23f;
accelData.y = 0.45f;
accelData.z = -0.67f;
```

---

### `bno055_euler_float_t`
A structure representing Euler angle output data in floating-point precision.

#### Fields
- `h`: Heading (yaw) angle in degrees.
- `r`: Roll angle in degrees.
- `p`: Pitch angle in degrees.

#### Example
```cpp
bno055_euler_float_t eulerData;
eulerData.h = 90.0f;
eulerData.r = -45.0f;
eulerData.p = 10.0f;
```

---

## Licensing

This file should be used in compliance with the licensing terms of the project it belongs to. The structures are designed for BNO055 sensor integration and should adhere to the sensor's usage guidelines.

---
**Header Guard**: 
```cpp
#ifndef BNO055_STRUCT_H
#define BNO055_STRUCT_H
```
Ensure this file is included only once during compilation.

# DataSaver Documentation

## Overview

The `DataSaver` namespace provides functionality for saving and loading various types of data, including calibration data, LIDAR scan data, BNO055 sensor data, and images. These utilities are designed to simplify data logging and retrieval for robotics applications.

---

## Functions

### `bool saveData`
```cpp
bool saveData(const std::string& filePath, const uint8_t calibData[22], bool append = true);
```
Saves calibration data to a file.

- **Parameters**:
  - `filePath`: Path to the file where the data will be saved.
  - `calibData`: Array containing 22 bytes of calibration data.
  - `append`: If `true`, appends the data to the file (default is `true`).

- **Returns**:
  - `true` if the data was saved successfully, `false` otherwise.

---

### `bool loadData`
```cpp
bool loadData(const std::string& filePath, uint8_t calibData[22]);
```
Loads calibration data from a file.

- **Parameters**:
  - `filePath`: Path to the file from which the data will be loaded.
  - `calibData`: Array to store the 22 bytes of loaded calibration data.

- **Returns**:
  - `true` if the data was loaded successfully, `false` otherwise.

---

### `bool saveLogData`
```cpp
bool saveLogData(const std::string& filePath, 
                 const std::vector<lidarController::NodeData>& scanData, 
                 const bno055_accel_float_t& accel_data, 
                 const bno055_euler_float_t& euler_data, 
                 const cv::Mat& image, 
                 bool append = true);
```
Saves log data including LIDAR scans, accelerometer data, Euler data, and an image.

- **Parameters**:
  - `filePath`: Path to the file where the log data will be saved.
  - `scanData`: Vector of LIDAR scan data.
  - `accel_data`: Accelerometer data (`bno055_accel_float_t`).
  - `euler_data`: Euler angle data (`bno055_euler_float_t`).
  - `image`: Image (`cv::Mat`) to be saved.
  - `append`: If `true`, appends the log data to the file (default is `true`).

- **Returns**:
  - `true` if the log data was saved successfully, `false` otherwise.

---

### `bool loadLogData`
```cpp
bool loadLogData(const std::string& filePath, 
                 std::vector<std::vector<lidarController::NodeData>>& allScanData, 
                 std::vector<bno055_accel_float_t>& allAccelData, 
                 std::vector<bno055_euler_float_t>& allEulerData, 
                 std::vector<cv::Mat>& allImages);
```
Loads log data including LIDAR scans, accelerometer data, Euler data, and images.

- **Parameters**:
  - `filePath`: Path to the file from which the log data will be loaded.
  - `allScanData`: Vector to store multiple sets of LIDAR scan data.
  - `allAccelData`: Vector to store accelerometer data.
  - `allEulerData`: Vector to store Euler angle data.
  - `allImages`: Vector to store images (`cv::Mat`).

- **Returns**:
  - `true` if the log data was loaded successfully, `false` otherwise.

---

## Namespace: `DataSaver`

The functions are encapsulated within the `DataSaver` namespace to avoid name conflicts and improve organization.

---

## Licensing

This file is part of a robotics project and should be used in accordance with its licensing terms.

---
**Header Guard**:
```cpp
#ifndef DATASAVER_H
#define DATASAVER_H
```N    
Ensure this file is included only once during compilation.
# Direction Header Documentation

## Overview

This header file defines enums and functions for managing cardinal and relative directions, including utilities for turning and converting directions to string or heading values.

---

## Enums

### `Direction`
Represents the four cardinal directions.

#### Values:
- `NORTH`
- `EAST`
- `SOUTH`
- `WEST`

---

### `RelativeDirection`
Represents relative movement directions.

#### Values:
- `FRONT`
- `BACK`
- `LEFT`
- `RIGHT`

---

### `TurnDirection`
Represents the possible turn directions.

#### Values:
- `CLOCKWISE`
- `COUNTER_CLOCKWISE`
- `UNKNOWN`

---

## Functions

### `Direction calculateRelativeDirection`
```cpp
Direction calculateRelativeDirection(Direction currentDirection, RelativeDirection relativeMove);
```

Calculates the new direction based on the current direction and a relative move.

#### Parameters:
- `currentDirection`: The current cardinal direction (`Direction`).
- `relativeMove`: The relative move to apply (`RelativeDirection`).

#### Returns:
- A `Direction` representing the new cardinal direction.

#### Example:
```cpp
Direction newDir = calculateRelativeDirection(NORTH, RIGHT);
// newDir will be EAST
```

---

### `std::string directionToString`
```cpp
std::string directionToString(Direction direction);
```

Converts a `Direction` enum value to a human-readable string.

#### Parameters:
- `direction`: A `Direction` enum value.

#### Returns:
- A string representation of the direction (e.g., "NORTH", "EAST").

#### Example:
```cpp
std::string dirStr = directionToString(WEST);
// dirStr will be "WEST"
```

---

### `float directionToHeading`
```cpp
float directionToHeading(Direction direction);
```

Converts a `Direction` enum value to a heading angle in degrees.

#### Parameters:
- `direction`: A `Direction` enum value.

#### Returns:
- A float representing the heading in degrees:
  - `NORTH` → 0°
  - `EAST` → 90°
  - `SOUTH` → 180°
  - `WEST` → 270°

#### Example:
```cpp
float heading = directionToHeading(SOUTH);
// heading will be 180.0f
```

---

## Licensing

This file is part of a robotics or navigation project. Use in compliance with the project's licensing terms.

---

**Header Guard**:
```cpp
#ifndef DIRECTION_H
#define DIRECTION_H
```

Prevents multiple inclusions during compilation.

# I2C Master Library Documentation

This documentation provides an overview of the `i2c_master` library, designed to manage I2C communication between a master device and a slave device. The library includes memory mapping, command structures, and utility functions for sending and receiving data.

## Namespace: `i2c_slave_mem_addr`
Defines memory layout and sizes for the I2C slave's memory map.

### Constants:
- **`MEM_SIZE`**: Total memory size (256 bytes).
- **`COMMAND_SIZE`**: Command register size (1 byte).
- **`STATUS_SIZE`**: Status register size (1 byte).
- **`GYRO_OFFSET_SIZE`**: Gyroscope offset data size (6 bytes).
- **`ACCEL_OFFSET_SIZE`**: Accelerometer offset data size (8 bytes).
- **`MAG_OFFSET_SIZE`**: Magnetometer offset data size (8 bytes).
- **`BNO055_CALIB_SIZE`**: Total calibration data size (22 bytes).
- **`ACCEL_DATA_SIZE`**: Accelerometer data size (12 bytes).
- **`EULER_ANGLE_SIZE`**: Euler angle data size (12 bytes).
- **`BNO055_INFO_SIZE`**: Total sensor info size (24 bytes).
- **`MOTOR_PERCENT_SIZE`**: Motor speed as float (4 bytes).
- **`STEERING_PERCENT_SIZE`**: Steering angle as float (4 bytes).
- **`MOVEMENT_INFO_SIZE`**: Combined movement info size (8 bytes).
- **`LOG_SIZE`**: Size of each log entry (1 byte).
- **`LOGS_BUFFER_SIZE`**: Total logs buffer size (256 bytes).

### Memory Map Addresses:
- **`COMMAND_ADDR`**: Start address of command data.
- **`STATUS_ADDR`**: Start address of status data.
- **`BNO055_CALIB_ADDR`**: Start address of calibration data.
- **`BNO055_INFO_ADDR`**: Start address of sensor info.
- **`MOVEMENT_INFO_ADDR`**: Start address of movement info.
- **`LOG_ADDR`**: Start address of log data.

### Static Assertions:
Ensures memory allocation fits within the defined buffer size.

---

## Enums

### `Command`
Defines available commands for the I2C slave device:
- **`NO_COMMAND (0x00)`**: No operation.
- **`RESTART (0x01)`**: Restart the slave device.
- **`CALIB_NO_OFFSET (0x02)`**: Calibrate without offsets.
- **`CALIB_WITH_OFFSET (0x03)`**: Calibrate with offsets.
- **`SKIP_CALIB (0x04)`**: Skip calibration.

---

## Functions

### Initialization and Setup
#### `int i2c_master_init(uint8_t slave_address)`
Initializes communication with the I2C slave device.
- **Parameters**:
  - `slave_address`: 7-bit address of the I2C slave device.
- **Returns**: File descriptor for the I2C device, or `-1` on failure.

---

### Command and Data Transmission
#### `void i2c_master_send_command(int fd, uint8_t command)`
Sends a command to the I2C slave device.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `command`: Command byte.

#### `void i2c_master_send_data(int fd, uint8_t reg, uint8_t *data, uint8_t len)`
Sends data to a specific register on the I2C slave device.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `reg`: Register address to write to.
  - `data`: Pointer to the data buffer.
  - `len`: Length of the data buffer.

#### `void i2c_master_read_data(int fd, uint8_t reg, uint8_t *data, uint8_t len)`
Reads data from a specific register on the I2C slave device.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `reg`: Register address to read from.
  - `data`: Pointer to the buffer for storing the data.
  - `len`: Number of bytes to read.

---

### Logging Functions
#### `void i2c_master_read_logs(int fd, uint8_t *logs)`
Reads log data from the I2C slave using the default size.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `logs`: Pointer to the buffer for storing logs.

#### `void i2c_master_read_logs(int fd, uint8_t *logs, size_t len)`
Reads log data with a specified size.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `logs`: Pointer to the buffer for storing logs.
  - `len`: Number of bytes to read.

#### `void i2c_master_print_logs(uint8_t *logs, size_t len)`
Prints logs in a human-readable format.
- **Parameters**:
  - `logs`: Pointer to the log data buffer.
  - `len`: Length of the log buffer.

---

### Reading Specific Data
#### `void i2c_master_read_command(int fd, uint8_t *command)`
Reads the command byte from the slave.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `command`: Pointer to the buffer for storing the command byte.

#### `void i2c_master_read_status(int fd, uint8_t *status)`
Reads the status byte from the slave.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `status`: Pointer to the buffer for storing the status byte.

#### `void i2c_master_read_bno055_calibration(int fd, uint8_t *calibration_data)`
Reads calibration data from the BNO055 sensor.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `calibration_data`: Pointer to the buffer for storing calibration data.

#### `void i2c_master_read_bno055_info(int fd, uint8_t *info_data)`
Reads sensor data (acceleration and Euler angles).
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `info_data`: Pointer to the buffer for storing sensor data.

#### `void i2c_master_read_movement_info(int fd, uint8_t *movement_info)`
Reads movement information (motor and steering percentages).
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `movement_info`: Pointer to the buffer for storing movement info.

#### `void i2c_master_read_bno055_accel_and_euler(int fd, bno055_accel_float_t *accel_data, bno055_euler_float_t *euler_data)`
Reads both acceleration and Euler angle data from the BNO055.
- **Parameters**:
  - `fd`: File descriptor for the I2C device.
  - `accel_data`: Pointer to the structure for storing acceleration data.
  - `euler_data`: Pointer to the structure for storing Euler angle data.

---

## Conclusion

The `i2c_master` library provides comprehensive support for I2C communication with a focus on sensor data retrieval, logging, and calibration. The modular design ensures easy integration into larger systems.
# Image Processor Library Documentation

This document provides an overview of the `image_processor` library, which uses OpenCV for image processing tasks such as detecting lines, identifying blocks, and processing specific color ranges.

---

## Constants

### General
- **`CROP_PERCENT`**: Ratio to crop the image for processing (50%).

### Color Ranges
Defined as `cv::Scalar` values for lower and upper HSV bounds to detect specific colors:
- **Blue Line**: 
  - `lowerBlueLine = (100, 100, 150)`
  - `upperBlueLine = (140, 220, 220)`
- **Orange Line**: 
  - `lowerOrangeLine = (6, 120, 180)`
  - `upperOrangeLine = (16, 230, 255)`
- **Red Line**: 
  - `lowerRed1Light = (0, 135, 160)`
  - `upperRed1Light = (2, 205, 255)`
  - `lowerRed2Light = (175, 135, 160)`
  - `upperRed2Light = (180, 205, 255)`
- **Green Line**: 
  - `lowerGreen1Light = (55, 70, 120)`
  - `upperGreen1Light = (84, 175, 195)`
  - `lowerGreen2Light = (84, 175, 195)`
  - `upperGreen2Light = (84, 175, 195)`
- **Pink**: 
  - `lowerPinkLight = (165, 244, 200)`
  - `upperPinkLight = (171, 255, 255)`

### Minimum Contour Areas
- **`minBlueLineArea = 37`**
- **`minOrangeLineArea = 37`**
- **`minRedLineArea = 300`**
- **`minGreenLineArea = 300`**

---

## Enums

### `enum class Color`
Defines the color categories used for block detection:
- **`BLUE`**
- **`ORANGE`**
- **`RED`**
- **`GREEN`**
- **`PINK`**

---

## Structs

### `struct Block`
Represents a detected block in the image.
- **`int x`**: X-coordinate of the block.
- **`int y`**: Y-coordinate of the block.
- **`int lowestY`**: Lowest Y-coordinate (for vertical alignment).
- **`int size`**: Size of the block.
- **`Color color`**: Detected color of the block.

### `struct ImageProcessingResult`
Stores the results of the image processing operations.
- **`int blueLineY`**: Y-coordinate of the blue line.
- **`int blueLineSize`**: Size of the blue line.
- **`int orangeLineY`**: Y-coordinate of the orange line.
- **`int orangeLineSize`**: Size of the orange line.
- **`std::vector<Block> blocks`**: Detected blocks.
- **`int pinkX`**: X-coordinate of the pink region.
- **`int pinkY`**: Y-coordinate of the pink region.
- **`int pinkSize`**: Size of the pink region.

---

## Functions

### Utility Functions
#### `std::vector<cv::Point> getCoordinates(const cv::Mat &mask)`
Extracts the coordinates of non-zero pixels in a mask.
- **Parameters**:
  - `mask`: Binary image mask.
- **Returns**: Vector of points with non-zero values.

#### `int getAverageX(const std::vector<cv::Point> &coordinates)`
Calculates the average X-coordinate.
- **Parameters**:
  - `coordinates`: Vector of points.
- **Returns**: Average X-coordinate.

#### `int getAverageY(const std::vector<cv::Point> &coordinates)`
Calculates the average Y-coordinate.
- **Parameters**:
  - `coordinates`: Vector of points.
- **Returns**: Average Y-coordinate.

#### `std::tuple<cv::Point, double, cv::Point> getCentroidAndArea(const std::vector<cv::Point> &contour)`
Calculates the centroid and area of a given contour.
- **Parameters**:
  - `contour`: Vector of points representing a contour.
- **Returns**: Tuple containing:
  - Centroid point.
  - Area of the contour.
  - Additional point for extended centroid information.

---

### Image Processing
#### `ImageProcessingResult processImage(const cv::Mat &image)`
Processes the input image to detect lines, blocks, and colored regions.
- **Parameters**:
  - `image`: Input image.
- **Returns**: Processed results in an `ImageProcessingResult` structure.

#### `cv::Mat filterAllColors(const cv::Mat &image)`
Applies color filtering to detect and highlight all defined color ranges.
- **Parameters**:
  - `image`: Input image.
- **Returns**: Processed image with color filters applied.

---

### Visualization
#### `cv::Mat drawImageProcessingResult(const ImageProcessingResult &result, cv::Mat &image)`
Draws the processing results (lines, blocks, and pink regions) on the input image.
- **Parameters**:
  - `result`: Processing results to visualize.
  - `image`: Input image to draw on.
- **Returns**: Image with visualizations.

---

### Conversion
#### `float pixelToAngle(int pixelX, int imageWidth, int centerOffset, float fov)`
Converts a pixel position to an angle relative to the image's field of view (FoV).
- **Parameters**:
  - `pixelX`: X-coordinate of the pixel.
  - `imageWidth`: Width of the image.
  - `centerOffset`: Offset for the center point.
  - `fov`: Field of view of the camera.
- **Returns**: Angle in degrees.

---

## Conclusion
The `image_processor` library provides comprehensive tools for color-based image analysis and line/block detection. Its modular structure supports real-time processing and visualization for robotics and computer vision applications.
# LibCamera Library Documentation

This documentation details the functionality of the `LibCamera` class, which provides a C++ interface to manage and interact with cameras using the `libcamera` library.

---

## Includes and Dependencies
The library depends on the following headers and libraries:
- **Standard Libraries**:
  - `<atomic>`: For atomic operations.
  - `<iomanip>`: For formatted output.
  - `<iostream>`: For input/output operations.
  - `<limits.h>`: For defining data type limits.
  - `<memory>`: For smart pointers.
  - `<stdint.h>`: For fixed-width integer types.
  - `<string>`: For string manipulation.
  - `<vector>`: For dynamic arrays.
  - `<unordered_map>`: For hash tables.
  - `<queue>`: For FIFO data structure.
  - `<sstream>`: For string streams.
  - `<sys/mman.h>`: For memory management.
  - `<unistd.h>`: For POSIX APIs.
  - `<time.h>`: For time operations.
  - `<mutex>`: For thread synchronization.

- **Libcamera Libraries**:
  - `<libcamera/controls.h>`: For camera control operations.
  - `<libcamera/control_ids.h>`: For control ID management.
  - `<libcamera/property_ids.h>`: For accessing camera properties.
  - `<libcamera/libcamera.h>`: Core libcamera library.
  - `<libcamera/camera.h>`: For camera management.
  - `<libcamera/camera_manager.h>`: For managing camera instances.
  - `<libcamera/framebuffer_allocator.h>`: For allocating frame buffers.
  - `<libcamera/request.h>`: For request handling.
  - `<libcamera/stream.h>`: For stream management.
  - `<libcamera/formats.h>`: For handling data formats.
  - `<libcamera/transform.h>`: For image transformations.

---

## Structs

### `LibcameraOutData`
Represents output data from the camera.
- **`uint8_t *imageData`**: Pointer to the image data.
- **`uint32_t size`**: Size of the image data in bytes.
- **`uint64_t request`**: Identifier for the request.

---

## Class: `LibCamera`

### Overview
The `LibCamera` class is responsible for initializing, configuring, and interacting with a camera device. It supports operations such as capturing frames, setting controls, and managing camera streams.

---

### Public Methods

#### Constructor and Destructor
- **`LibCamera()`**: Default constructor.
- **`~LibCamera()`**: Destructor to clean up resources.

#### Camera Initialization and Configuration
- **`int initCamera()`**: Initializes the camera.
- **`void configureStill(int width, int height, PixelFormat format, int buffercount, Orientation orientation)`**:
  Configures the camera for still image capture.
  - **Parameters**:
    - `width`: Desired image width.
    - `height`: Desired image height.
    - `format`: Pixel format for the images.
    - `buffercount`: Number of frame buffers.
    - `orientation`: Camera orientation.

#### Camera Control and State
- **`int startCamera()`**: Starts the camera capture session.
- **`int resetCamera(int width, int height, PixelFormat format, int buffercount, Orientation orientation)`**:
  Resets the camera with new configuration parameters.
- **`void stopCamera()`**: Stops the camera capture session.
- **`void closeCamera()`**: Closes and releases the camera.

#### Frame Handling
- **`bool readFrame(LibcameraOutData *frameData)`**:
  Reads a frame from the camera.
  - **Parameters**:
    - `frameData`: Pointer to the output frame data structure.
  - **Returns**: `true` if successful, `false` otherwise.
- **`void returnFrameBuffer(LibcameraOutData frameData)`**:
  Returns the frame buffer to the camera for reuse.

#### Control Management
- **`void set(ControlList controls)`**: Sets camera controls using a `ControlList`.

#### Stream Management
- **`Stream *VideoStream(uint32_t *w, uint32_t *h, uint32_t *stride) const`**:
  Retrieves the video stream dimensions.
  - **Parameters**:
    - `w`: Pointer to store the width.
    - `h`: Pointer to store the height.
    - `stride`: Pointer to store the stride.
  - **Returns**: Pointer to the video stream.

#### Camera Information
- **`char *getCameraId()`**: Retrieves the unique identifier of the camera.

---

### Private Methods

#### Capture and Request Management
- **`int startCapture()`**: Begins capturing frames.
- **`int queueRequest(Request *request)`**:
  Queues a request for processing.
  - **Parameters**:
    - `request`: Pointer to the request.
- **`void requestComplete(Request *request)`**:
  Handles a completed request.
- **`void processRequest(Request *request)`**:
  Processes the request and extracts data.

#### Stream Utility
- **`void StreamDimensions(Stream const *stream, uint32_t *w, uint32_t *h, uint32_t *stride) const`**:
  Gets the dimensions of a specific stream.
  - **Parameters**:
    - `stream`: Pointer to the stream.
    - `w`: Pointer to store the width.
    - `h`: Pointer to store the height.
    - `stride`: Pointer to store the stride.

---

### Private Members
- **Camera and Manager**:
  - `unsigned int cameraIndex_`: Index of the camera.
  - `uint64_t last_`: Timestamp of the last operation.
  - `std::unique_ptr<CameraManager> cm`: Camera manager instance.
  - `std::shared_ptr<Camera> camera_`: Shared pointer to the camera.
  - `std::unique_ptr<CameraConfiguration> config_`: Configuration for the camera.
  - `std::unique_ptr<FrameBufferAllocator> allocator_`: Allocator for frame buffers.

- **Request Management**:
  - `std::vector<std::unique_ptr<Request>> requests_`: List of requests.
  - `std::queue<Request *> requestQueue`: Queue of requests.

- **Buffer Management**:
  - `std::map<int, std::pair<void *, unsigned int>> mappedBuffers_`: Mapped buffers for frame data.

- **Streams**:
  - `Stream *viewfinder_stream_`: Pointer to the viewfinder stream.

- **Controls and Synchronization**:
  - `ControlList controls_`: Current controls.
  - `std::mutex control_mutex_`: Mutex for control operations.
  - `std::mutex camera_stop_mutex_`: Mutex for stopping the camera.
  - `std::mutex free_requests_mutex_`: Mutex for managing free requests.

- **Camera Metadata**:
  - `std::string cameraId`: Identifier of the camera.

---

## Conclusion
The `LibCamera` class encapsulates the complexities of working with `libcamera`, offering a high-level interface for camera operations. It supports advanced features such as request management, stream handling, and control configuration, making it suitable for applications requiring precise camera control and data capture.

# LidarController.h Documentation

## Overview

The `LidarController` class provides functionality for controlling and acquiring data from a SLAMTEC LIDAR device. It handles initialization, scanning, data retrieval, and shutdown procedures for the LIDAR sensor.

## Class: `LidarController`

### Constructor: `LidarController(const char* serialPort = "/dev/ttyAMA0", int baudRate = 460800)`
- **Description**: Initializes the `LidarController` object with the provided serial port and baud rate.
- **Parameters**:
  - `serialPort` (default `"/dev/ttyAMA0"`) - The serial port path used to communicate with the LIDAR device.
  - `baudRate` (default `460800`) - Baud rate for communication with the LIDAR device.

### Destructor: `~LidarController()`
- **Description**: Cleans up resources and shuts down the LIDAR when the object is destroyed.

### Method: `bool initialize()`
- **Description**: Initializes the LIDAR driver and establishes a connection with the device.
- **Returns**: `true` if the initialization is successful, `false` otherwise.

### Method: `bool startScanning()`
- **Description**: Starts scanning using the LIDAR. This initiates the motor spin and begins data acquisition.
- **Returns**: `true` if the scan starts successfully, `false` otherwise.

### Method: `void stopScanning()`
- **Description**: Stops the LIDAR scanning process and halts the motor.

### Method: `std::vector<NodeData> getScanData()`
- **Description**: Retrieves scan data from the LIDAR, including angles and distances.
- **Returns**: A vector of `NodeData` containing the scan results.

### Method: `void shutdown()`
- **Description**: Shuts down the LIDAR, stops scanning, and cleans up all resources.

### Method: `static void printScanData(const std::vector<NodeData>& nodeDataVector)`
- **Description**: Prints the LIDAR scan data to the console.
- **Parameters**:
  - `nodeDataVector` - A vector of `NodeData` containing the scan results to be printed.

## Private Members

- `sl::ILidarDriver* lidarDriver` - A pointer to the LIDAR driver instance.
- `sl::IChannel* serialChannel` - A pointer to the communication channel.
- `const char* serialPort` - The serial port used for LIDAR connection.
- `int baudRate` - Baud rate for the communication.

## Dependencies

- `sl_lidar.h` and `sl_lidar_driver.h` - Headers for interacting with the SLAMTEC LIDAR.
- `lidar_struct.h` - A header containing the `NodeData` structure and other necessary definitions.

## Notes
- The `LidarController` class requires a valid SLAMTEC LIDAR device and an appropriate serial port for communication.
# LIDAR Data Processor Documentation

## Overview
The `LIDAR Data Processor` is a C++ library that processes LIDAR data for detecting and analyzing traffic lights, parking zones, and walls. It works by converting raw LIDAR data into a 2D image for further processing using OpenCV techniques. The library can also detect lines, calculate angles, and process data to determine directions for navigation.

## Structures

### `BlockInfo`
Contains information about each detected block in the LIDAR scan.

- `float angle`: The angle of the detected block in degrees.
- `int size`: The size of the block.
- `Color color`: The color of the traffic light associated with the block.

### `ProcessedTrafficLight`
Contains information about each detected traffic light.

- `cv::Point point`: The location of the block.
- `int size`: The size of the block.
- `Color color`: The color of the traffic light.

## Functions

### `cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData> &data, int width, int height, float scale)`
Converts raw LIDAR data into a grayscale OpenCV image. The image will be scaled based on the provided scale factor.

- **Parameters**:
  - `data`: A vector containing the LIDAR data points.
  - `width`: The width of the output image.
  - `height`: The height of the output image.
  - `scale`: A scaling factor to adjust the image resolution.
  
- **Returns**: A grayscale OpenCV image.

### `float toMeter(int scale, double lidarDistance)`
Converts a LIDAR distance into meters, based on the scaling factor.

- **Parameters**:
  - `scale`: The scale factor used for the conversion.
  - `lidarDistance`: The raw distance from the LIDAR.
  
- **Returns**: The distance in meters.

### `std::vector<cv::Vec4i> detectLines(const cv::Mat &binaryImage)`
Detects lines in a binary image using the Hough Line Transform.

- **Parameters**:
  - `binaryImage`: The binary image (after thresholding) to detect lines in.

- **Returns**: A vector of lines represented by four values: `(x1, y1, x2, y2)` for each detected line.

### `double lineLength(const cv::Vec4i& line)`
Calculates the length of a line from the vector `(x1, y1, x2, y2)`.

- **Parameters**:
  - `line`: A vector representing the line with its endpoints.

- **Returns**: The length of the line.

### `double calculateAngle(const cv::Vec4i &line)`
Calculates the angle of a line in degrees.

- **Parameters**:
  - `line`: A vector representing the line with its endpoints.

- **Returns**: The angle of the line in degrees.

### `double pointLinePerpendicularDistance(const cv::Point2f& pt, const cv::Vec4i& line)`
Calculates the perpendicular distance from a point to a line.

- **Parameters**:
  - `pt`: A point.
  - `line`: A line represented by a vector of four points.

- **Returns**: The perpendicular distance from the point to the line.

### `double pointLinePerpendicularDirection(const cv::Point2f& pt, const cv::Vec4i& line)`
Calculates the perpendicular direction from a point to a line.

- **Parameters**:
  - `pt`: A point.
  - `line`: A line represented by a vector of four points.

- **Returns**: The perpendicular direction.

### `double pointToLineSegmentDistance(const cv::Point2f& P, const cv::Vec4i& lineSegment)`
Calculates the perpendicular distance from a point to a line segment.

- **Parameters**:
  - `P`: The point.
  - `lineSegment`: A line segment represented by four points.

- **Returns**: The perpendicular distance from the point to the line segment.

### `cv::Vec4i extendLine(const cv::Vec4i& line, double factor)`
Extends a line by a given factor.

- **Parameters**:
  - `line`: A vector representing the line.
  - `factor`: The factor by which to extend the line.

- **Returns**: The extended line.

### `bool areLinesAligned(const cv::Vec4i& line1, const cv::Vec4i& line2, double angleThreshold, double collinearThreshold)`
Checks if two lines are aligned and collinear within specified thresholds.

- **Parameters**:
  - `line1`: The first line.
  - `line2`: The second line.
  - `angleThreshold`: The angle threshold for alignment.
  - `collinearThreshold`: The threshold for collinearity.

- **Returns**: `true` if the lines are aligned and collinear, otherwise `false`.

### `std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold = 12.0, double collinearThreshold = 10.0)`
Combines aligned and collinear lines into a single line.

- **Parameters**:
  - `lines`: A vector of lines.
  - `angleThreshold`: The threshold for angle alignment (default 12.0 degrees).
  - `collinearThreshold`: The threshold for collinearity (default 10.0).

- **Returns**: A vector of combined lines.

### `std::vector<Direction> analyzeWallDirection(const std::vector<cv::Vec4i>& combinedLines, float gyroYaw, const cv::Point& center)`
Analyzes the directions of walls based on the combined lines and gyro data.

- **Parameters**:
  - `combinedLines`: A vector of combined lines.
  - `gyroYaw`: The yaw value from the gyro sensor.
  - `center`: The center point of the analysis.

- **Returns**: A vector of directions (NORTH, EAST, SOUTH, WEST).

### `std::vector<cv::Point> detectTrafficLight(const cv::Mat& binaryImage, const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, TurnDirection turnDirection, Direction direction)`
Detects traffic light positions in the binary image based on the lines and directions.

- **Parameters**:
  - `binaryImage`: The binary image.
  - `combinedLines`: A vector of combined lines.
  - `wallDirections`: A vector of wall directions.
  - `turnDirection`: The turn direction (LEFT, RIGHT, etc.).
  - `direction`: The current direction.

- **Returns**: A vector of points representing the traffic light locations.

### `std::vector<cv::Vec4i> detectParkingZone(const cv::Mat& binaryImage, const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, TurnDirection turnDirection, Direction direction)`
Detects parking zones in the binary image based on the lines and directions.

- **Parameters**:
  - `binaryImage`: The binary image.
  - `combinedLines`: A vector of combined lines.
  - `wallDirections`: A vector of wall directions.
  - `turnDirection`: The turn direction.
  - `direction`: The current direction.

- **Returns**: A vector of parking zone lines.

### `std::vector<ProcessedTrafficLight> processTrafficLight(const std::vector<cv::Point>& trafficLightPoints, const std::vector<BlockInfo>& blockAngles, const cv::Point& center)`
Processes the detected traffic lights and classifies them.

- **Parameters**:
  - `trafficLightPoints`: A vector of points where traffic lights are located.
  - `blockAngles`: A vector of `BlockInfo` containing block angle data.
  - `center`: The center point of the analysis.

- **Returns**: A vector of `ProcessedTrafficLight` containing processed traffic lights.

### `TurnDirection lidarDetectTurnDirection(const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, Direction direction)`
Detects the turn direction based on the combined lines and wall directions.

- **Parameters**:
  - `combinedLines`: A vector of combined lines.
  - `wallDirections`: A vector of wall directions.
  - `direction`: The current direction.

- **Returns**: The detected turn direction.

### `void drawAllLines(cv::Mat &outputImage, const std::vector<cv::Vec4i> &lines, const std::vector<Direction> &wallDirections)`
Draws all the lines on the output image.

- **Parameters**:
  - `outputImage`: The output image to draw on.
  - `lines`: A vector of lines.
  - `wallDirections`: A vector of wall directions.

### `void drawTrafficLights(cv::Mat& outputImage, const std::vector<ProcessedTrafficLight>& processedBlocks)`
Draws the processed traffic lights on the output image.

- **Parameters**:
  - `outputImage`: The output image to draw on.
  - `processedBlocks`: A vector of processed traffic lights.
# LIDAR Struct Documentation

## Overview
The `LIDAR Struct` is a simple C++ structure that holds data for each LIDAR scan node. It stores information about the angle and distance of a detected point in the LIDAR scan.

## Structures

### `NodeData`
This structure represents a single LIDAR scan node.

- `float angle`: The angle of the detected point in the scan, typically in degrees. This indicates the orientation of the point relative to the LIDAR's current position.
- `float distance`: The distance of the detected point from the LIDAR, typically in meters. This represents how far away the point is from the LIDAR sensor.

## Usage
The `NodeData` structure is commonly used to store and process individual points from LIDAR scans, which can then be used for visualization, navigation, or other analysis tasks.
