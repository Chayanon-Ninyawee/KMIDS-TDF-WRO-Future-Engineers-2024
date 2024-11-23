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

- It is mandatory to mark “mingw-developer-tool” (provide some necessary developer tools) and “mingw32-base” (basic MinGW installation). As the image below, there are multiple versions of GNU Compilers. It depends on your needs, and which compiler you want to install. ![Compilers](https://files.codingninjas.in/article_images/custom-upload-1683458541.webp)

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
##### - Cloning the SDK locally
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
##### - Setup with the Raspberry Pi Pico SDK as a submodule
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
##### - Set up with automatic download from GitHub
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
##### - Cloning the SDK locally, but without copying ```pico_sdk_import.cmake```
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