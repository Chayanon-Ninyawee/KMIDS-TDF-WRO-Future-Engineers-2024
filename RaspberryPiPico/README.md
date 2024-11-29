# How To Install CMake for Window 10/11
- Go to the [website](https://cmake.org/download/) and download the latest version.
  
- Double click to run the installer.

- When asked for selecting option, select “Add CMake to the system PATH for all users”. Then click install and finish button. 

![Option](https://perso.uclouvain.be/allan.barrea/opencv/_images/cmake_add_path.png)

- Add the path to the environment variables. You have to open “Edit the system environment variables”. ![Environment Variable](https://files.codingninjas.in/article_images/custom-upload-1683458579.webp)

- Click “Environment Variables”.

![Button](https://files.codingninjas.in/article_images/custom-upload-1683458591.webp)

- Go to “System Variables” and click on “path”. 

![Path](https://files.codingninjas.in/article_images/custom-upload-1683458604.webp)

- Add the path of the CMake folder along with its bin file. 


- To check installation, open the command prompt and type "cmake"


# How To Install GCC Compiler for Windows 10/11
- Open the [link](https://sourceforge.net/projects/mingw-w64/) and click the “Download” button to download the MinGW-w64 - for 32 and 64 bit Windows. ![download](https://files.codingninjas.in/article_images/custom-upload-1683458386.webp)

- After downloading, you must extract the zip file by right click and choose extract all option and install the folder and “mingw-get-setup” application. 

![zip folder](https://files.codingninjas.in/article_images/custom-upload-1683458487.webp)

- Click Install Button. 

![Install button](https://files.codingninjas.in/article_images/custom-upload-1683517444.png)

- It will appear Installation preferences. Keep this as default and don’t change anything. Click on the “Continue” button and it will move to the next page.

![Installation preferences](https://files.codingninjas.in/article_images/custom-upload-1683458514.webp)

- Wait downloading the “MinGW Installation Manager” as shown in the image below. ![downloading manager](https://files.codingninjas.in/article_images/custom-upload-1683458527.webp)

- It is mandatory to mark “mingw-developer-tool” (provide some necessary developer tools) and “mingw32-base” (basic MinGW installation). As the image below, there are multiple versions of GNU Compilers. It depends on your needs, and which compiler you want to install. 

![Compilers](https://files.codingninjas.in/article_images/custom-upload-1683458541.webp)

- Click on the “Apply Changes” to install all the libraries, header-files and modules of the GNU Compiler that selected. ![Apply Change](https://files.codingninjas.in/article_images/custom-upload-1683458554.webp)

- Add the path to the environment variables. You have to open “Edit the system environment variables”. ![Environment Variable](https://files.codingninjas.in/article_images/custom-upload-1683458579.webp)

- Click “Environment Variables”. 

![Button](https://files.codingninjas.in/article_images/custom-upload-1683458591.webp)

- Go to “System Variables” and click on “path”. 

![Path](https://files.codingninjas.in/article_images/custom-upload-1683458604.webp)

- Add the path of the MinGW folder along with its bin file. 

![Add path](https://files.codingninjas.in/article_images/custom-upload-1683458614.webp)

- It is good to go now. Open commander prompt and the image below should show the g++ version installed. 

![finish installation](https://files.codingninjas.in/article_images/custom-upload-1683458629.webp) 

# Raspberry Pi Pico SDK
## Get SDK code
The [master](https://github.com/raspberrypi/pico-sdk/tree/master/) branch of ```pico-sdk``` on GitHub contains the latest stable release of the SDK. If you need or want to test upcoming features, you can try the [develop](https://github.com/raspberrypi/pico-sdk/tree/develop/) branch instead.
## Visual (VS) code
- Install [the Raspberry Pi Pico Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) in VS Code.
## Installing and Setup
These instructions and steps are Linux-based only. For other platforms and detailed steps, we suggest you look at [Raspberry Pi Pico-Series C/C++ SDK](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf)
- Install CMake and GCC Complier for Linux
```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```
- There are multiple ways to set up project to point to use the Raspberry Pi Pico SDK:
### - Cloning the SDK locally
1. ```git clone``` Raspberry Pi Pico SDK from this [repository](https://github.com/raspberrypi/pico-sdk/tree/develop/?tab=readme-ov-file)
2. Copy [pico_sdk_import.cmake](https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake) from the SDK into project directory.
3. Set ```PICO_SDK_PATH``` to the SDK location in environment, or pass it (```-DPICO_SDK_PATH=```) to cmake later.
4. Setup a CMakeLists.txt like:
```
cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of project

```      
### - Setup with the Raspberry Pi Pico SDK as a submodule
1. Clone the SDK as a submodule ```called pico-sdk```
2. Setup a ```CMakeLists.txt``` like:
```
cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of project

```
### - Set up with automatic download from GitHub
1. Copy [pico_sdk_import.cmake](https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake) from the SDK into project directory
2. Setup a ```CMakeLists.txt``` like:
```
cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of project
```
### - Cloning the SDK locally, but without copying ```pico_sdk_import.cmake```
1. ```git clone``` from this [Raspberry Pi Pico SDK repository](https://github.com/raspberrypi/pico-sdk/tree/develop/?tab=readme-ov-file)
2. Setup a ```CMakeLists.txt``` like:
```
cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
include(/path/to/pico-sdk/pico_sdk_init.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of project

```
- Write  code using:
```
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    printf("Hello, world!\n");
    return 0;
}
```
- Then, add the following to ```CMakeLists.txt```:
```
add_executable(hello_world
    hello_world.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)
```
Note that this example uses the default UART for stdout. If you'd like to use the default USB, refer to the [hello-usb](https://github.com/raspberrypi/pico-examples/tree/master/hello_world/usb) example.

- Setup a CMake build directory. For example, if not using an IDE:
```
$ mkdir build
$ cd build
$ cmake ..
```
When building for a board other than the Raspberry Pi Pico, you should include ```-DPICO_BOARD=board_name``` in the cmake command, for example, ```cmake -DPICO_BOARD=pico2 ..``` or ```cmake -DPICO_BOARD=pico_w ..```. This ensures the SDK and build options are correctly configured for the specified board.

By specifying ```PICO_BOARD=<boardname>```, you define various compiler settings (such as default pin numbers for UART and other hardware). In some cases, it also enables the use of additional libraries (like wireless support for ```PICO_BOARD=pico_w```), which require hardware features available only on specific boards.
- Make your target from the build directory you created:
```
$ make hello_world
```
- You now have ```hello_world.elf``` to load via a debugger, or ```hello_world.uf2``` that can be installed and run on your Raspberry Pi Pico-series device via drag and drop.
# BNO055 Utils Documentation

## Overview
This header file contains utility functions and structures for working with the BNO055 sensor, a 9-axis motion tracking device. The file provides functions for initializing the sensor, calibrating it, and loading calibration offsets. It is designed for use with the Raspberry Pi Pico platform using the `pico/stdlib` library and the `bno055` sensor library.

## Constants

- `BNO055_POWER_MODE`: The power mode for the BNO055 sensor. Set to `BNO055_POWER_MODE_NORMAL`, which provides normal operating power consumption.
- `BNO055_OPERATION_MODE`: The operation mode for the BNO055 sensor. Set to `BNO055_OPERATION_MODE_NDOF` (Nine Degrees of Freedom), which enables the sensor to provide a complete set of motion and orientation data.

## Type Definitions

- `bno055_t`: A typedef for the `bno055` structure, which represents the BNO055 sensor instance.
- `bno055_gyro_offset_t`: A typedef for the structure that stores the gyroscope offset data for calibration.
- `bno055_accel_offset_t`: A typedef for the structure that stores the accelerometer offset data for calibration.
- `bno055_mag_offset_t`: A typedef for the structure that stores the magnetometer offset data for calibration.
- `bno055_accel_float_t`: A typedef for the structure that holds accelerometer data in floating-point format.
- `bno055_euler_float_t`: A typedef for the structure that holds Euler angle data (rotation angles) in floating-point format.

## Functions

### `int8_t bno055_initialize(bno055_t *bno, i2c_inst_t *i2c)`
- **Description**: Initializes the BNO055 sensor.
- **Parameters**:
  - `bno`: A pointer to the `bno055_t` structure representing the sensor instance.
  - `i2c`: An instance of the I2C interface to communicate with the sensor.
- **Return Value**: Returns `0` on successful initialization, or a negative value indicating an error.

### `int8_t bno055_calibrate(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset)`
- **Description**: Calibrates the BNO055 sensor. This function adjusts the sensor offsets for the gyroscope, accelerometer, and magnetometer.
- **Parameters**:
  - `gyroOffset`: A pointer to the `bno055_gyro_offset_t` structure containing the gyroscope offsets.
  - `accelOffset`: A pointer to the `bno055_accel_offset_t` structure containing the accelerometer offsets.
  - `magOffset`: A pointer to the `bno055_mag_offset_t` structure containing the magnetometer offsets.
- **Return Value**: Returns `0` on successful calibration, or a negative value indicating an error.

### `int8_t bno055_load_offset(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset)`
- **Description**: Loads previously stored calibration offsets into the BNO055 sensor.
- **Parameters**:
  - `gyroOffset`: A pointer to the `bno055_gyro_offset_t` structure containing the gyroscope offsets to be loaded.
  - `accelOffset`: A pointer to the `bno055_accel_offset_t` structure containing the accelerometer offsets to be loaded.
  - `magOffset`: A pointer to the `bno055_mag_offset_t` structure containing the magnetometer offsets to be loaded.
- **Return Value**: Returns `0` on successful offset loading, or a negative value indicating an error.

## Usage

The BNO055 sensor is initialized by calling `bno055_initialize()`, which sets up communication with the sensor over I2C. Calibration can be performed using the `bno055_calibrate()` function, and once the sensor is calibrated, offsets can be loaded with `bno055_load_offset()`.

This header file provides a foundation for working with the BNO055 sensor on platforms such as the Raspberry Pi Pico, and can be extended to provide additional functionality as needed.

# Debug Print Header Documentation

## Overview

This header file provides functionality for conditional debug logging in C programs. It allows for debug messages to be printed to the console and optionally appended to log files. The logging behavior is controlled through the `DEBUG` macro, enabling or disabling the logging based on the build configuration.

## Preprocessor Directives

### `#define DEBUG`

The `DEBUG` macro is used to enable or disable debug logging in the code. If `DEBUG` is defined, debug logging will be enabled. If not, the debug print macros will be excluded from the code to reduce overhead in production environments.

### `#ifndef NO_LOG`

This directive checks if the `NO_LOG` macro is defined. If `NO_LOG` is not defined, debug logs will be both printed to the console and appended to a log file. If `NO_LOG` is defined, debug messages will only be printed to the console and not logged.

## Debug Print Macro

### `#define DEBUG_PRINT(...)`

This macro is used to log debug messages. The behavior of the macro depends on the `DEBUG` and `NO_LOG` macros:

- If `DEBUG` is defined and `NO_LOG` is not defined:
    - The message is printed to the console using `printf`.
    - The message is then formatted into a buffer and appended to a log using the `append_logs` function, which writes the logs to persistent storage.
    
- If `DEBUG` is defined and `NO_LOG` is defined:
    - The message is only printed to the console using `printf`, without appending to logs.

- If `DEBUG` is not defined:
    - The macro does nothing, effectively removing the debug log statements from the code to optimize for production.

### Example Usage



# I2C Slave Utilities Header Documentation

## Overview

This header file defines utilities for managing an I2C slave interface. It includes functionality for managing memory space for various data structures, setting and retrieving commands, storing BNO055 sensor data, and appending logs in a rolling buffer. The memory layout and commands are well defined to interact with the I2C slave system.

## Namespace: `i2c_slave_mem_addr`

This namespace holds constants and macros to define memory sizes, addresses, and layout for the I2C slave memory.

### Constants

- `MEM_SIZE`: Total memory size allocated for the I2C slave. Set to 256 bytes.
- `COMMAND_SIZE`: Size of the command byte (1 byte).
- `STATUS_SIZE`: Size of the status byte (1 byte).
- `GYRO_OFFSET_SIZE`: Size of the gyro offset data structure.
- `ACCEL_OFFSET_SIZE`: Size of the accelerometer offset data structure.
- `MAG_OFFSET_SIZE`: Size of the magnetometer offset data structure.
- `BNO055_CALIB_SIZE`: Total size of BNO055 calibration data (sum of the gyro, accelerometer, and magnetometer offsets).
- `ACCEL_DATA_SIZE`: Size of the accelerometer data structure.
- `EULER_ANGLE_SIZE`: Size of the Euler angles data structure.
- `BNO055_INFO_SIZE`: Total size of BNO055 information (sum of accelerometer data and Euler angles).
- `MOTOR_PERCENT_SIZE`: Size of the motor percentage data (float).
- `STEERING_PERCENT_SIZE`: Size of the steering percentage data (float).
- `MOVEMENT_INFO_SIZE`: Total size of movement information (sum of motor and steering percentages).
- `LOG_SIZE`: Size of a single log entry (1 byte).
- `LOGS_BUFFER_SIZE`: Size of the log buffer (256 bytes).
- `COMMAND_ADDR`: Starting address for the command data.
- `STATUS_ADDR`: Address for status data.
- `BNO055_CALIB_ADDR`: Address for BNO055 calibration data.
- `BNO055_INFO_ADDR`: Address for BNO055 information data.
- `MOVEMENT_INFO_ADDR`: Address for movement information.
- `LOG_ADDR`: Address for log data.

A static assertion ensures that the total memory allocation fits within the defined memory size.

## Enum: `Command`

This enum defines the available commands for the I2C slave

# PWM Utilities Documentation

This header file defines a set of utility functions for controlling PWM (Pulse Width Modulation) on GPIO pins. These functions are used to configure PWM output for general purposes, servo motors, and motor drivers such as the L9110S.

## Header File Overview

```c
#ifndef PWM_UTILS_H
#define PWM_UTILS_H

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
```

This section includes the necessary libraries for controlling hardware clocks, GPIO pins, interrupts, and PWM functionality.

## Function Documentation

### `void setup_pwm(uint gpio_pin, float freq);`

**Description:**

Configures a GPIO pin for PWM output with the specified frequency.

**Parameters:**

- `gpio_pin`: GPIO pin number to be configured for PWM.
- `freq`: Desired PWM frequency in Hz.

**Details:**

This function assigns the given GPIO pin to a PWM slice and calculates the necessary clock divider to generate the desired frequency.

---

### `void enable_pwm(uint gpio_pin, float duty_cycle);`

**Description:**

Enables PWM output on a specified GPIO pin with a defined duty cycle.

**Parameters:**

- `gpio_pin`: GPIO pin number for PWM output.
- `duty_cycle`: Duty cycle as a fraction (0.0 - 1.0), where 1.0 represents 100%.

**Details:**

This function sets the PWM duty cycle to the specified value and enables PWM output on the given GPIO pin.

---

### `void disable_pwm(uint gpio_pin);`

**Description:**

Disables PWM output on a specified GPIO pin.

**Parameters:**

- `gpio_pin`: GPIO pin number where PWM is to be disabled.

**Details:**

This function turns off the PWM output by deactivating the PWM slice associated with the given GPIO pin.

---

### `void setup_servo(uint servo_pin, uint start_angle);`

**Description:**

Initializes a GPIO pin for servo control and sets the initial angle.

**Parameters:**

- `servo_pin`: GPIO pin number connected to the servo.
- `start_angle`: Initial angle of the servo in degrees (0 - 180).

**Details:**

This function sets up the PWM signal for a servo motor at a frequency of 50 Hz and positions the servo to the specified starting angle.

---

### `void set_servo_angle(uint servo_pin, uint angle);`

**Description:**

Sets the angle of a servo motor connected to a specified GPIO pin.

**Parameters:**

- `servo_pin`: GPIO pin number connected to the servo.
- `angle`: Desired angle in degrees (0 - 180).

**Details:**

This function adjusts the PWM duty cycle to move the servo to the desired angle. The angle is converted to the corresponding PWM signal.

---

### `void setup_L9110S_motor_driver(uint motorA_pin, uint motorB_pin);`

**Description:**

Configures two GPIO pins for use with an L9110S motor driver.

**Parameters:**

- `motorA_pin`: GPIO pin connected to the motor driver's A input.
- `motorB_pin`: GPIO pin connected to the motor driver's B input.

**Details:**

This function sets up the L9110S motor driver by configuring the GPIO pins for PWM output and initializing the duty cycle to 0.

---

### `void set_L9110S_motor_speed(uint motorA_pin, uint motorB_pin, float speed);`

**Description:**

Controls the speed and direction of a motor connected to an L9110S driver.

**Parameters:**

- `motorA_pin`: GPIO pin connected to the motor driver's A input.
- `motorB_pin`: GPIO pin connected to the motor driver's B input.
- `speed`: Motor speed as a fraction (-1.0 to 1.0), where positive values represent forward rotation and negative values represent reverse rotation.

**Details:**

This function adjusts the PWM duty cycle on the motor driver inputs to control motor speed and direction. A speed value of `0` stops the motor.

---

## End of File

```c
#endif // PWM_UTILS_H
```

This preprocessor directive ensures that the header file is included only once during compilation.
