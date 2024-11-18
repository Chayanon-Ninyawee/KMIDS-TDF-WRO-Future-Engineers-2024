## Dependencies
OpenCV needs other software libraries to work, and these must be installed first. Some of them come with the Raspberry Pi 64-bit operating system, and others might already be installed. To be safe, here's the full list. Only the latest packages are installed according to the procedure.
```
# check for updates
$ sudo apt-get update
$ sudo apt-get upgrade
# dependencies
$ sudo apt-get install build-essential cmake git unzip pkg-config
$ sudo apt-get install libjpeg-dev libpng-dev
$ sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
$ sudo apt-get install libgtk2.0-dev libcanberra-gtk* libgtk-3-dev
$ sudo apt-get install libgstreamer1.0-dev gstreamer1.0-gtk3
$ sudo apt-get install libgstreamer-plugins-base1.0-dev gstreamer1.0-gl
$ sudo apt-get install libxvidcore-dev libx264-dev
$ sudo apt-get install python3-dev python3-numpy python3-pip
$ sudo apt-get install libtbb2 libtbb-dev libdc1394-22-dev
$ sudo apt-get install libv4l-dev v4l-utils
$ sudo apt-get install libopenblas-dev libatlas-base-dev libblas-dev
$ sudo apt-get install liblapack-dev gfortran libhdf5-dev
$ sudo apt-get install libprotobuf-dev libgoogle-glog-dev libgflags-dev
$ sudo apt-get install protobuf-compiler
```
## Download OpenCV
Once all the third-party software is installed, you can download OpenCV itself. You'll need two packages: the basic version and the additional contributions. After downloading, simply unzip the files. Be careful with line wrapping in the command boxes. The two commands begin with ```wget``` and end with .```zip.```
```
# check your memory first
$ free -m
# you need at least a total of 5.8 GB!
# if not, enlarge your swap space as explained earlier
# download the latest version
$ cd ~
$ git clone --depth=1 https://github.com/opencv/opencv.git
$ git clone --depth=1 https://github.com/opencv/opencv_contrib.git
```
## Build Make
Before begin with the actual build of the library. You have to make a directory where all the build files can be located.
```
$ cd ~/opencv
$ mkdir build
$ cd build
```
In this step, you instruct CMake on what , where , and how to make OpenCV on your Raspberry Pi. There are several flags to consider, most of which will be familiar to you. One important line you’ll notice is -D WITH_QT=OFF, which disables Qt5 support. If you want to use Qt5 for the GUI, change this to -D WITH_QT=ON. We also save space by excluding any (Python) examples or tests.

Note that there should be spaces before the -D flags, not tabs. Also, the two dots at the end are no typo. It tells CMake where to find its CMakeLists.txt file (the main configuration file); one directory up.
```
$ cmake -D CMAKE_BUILD_TYPE=RELEASE \
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
$ make -j4
```
Hope that it will show the piture below: ![](https://qengineering.eu/images/CMakeSucces64_4_5_0.webp)
Install all the generated packages to the database of your system with the next commands.
```
$ sudo make install
$ sudo ldconfig
# cleaning (frees 300 KB)
$ make clean
$ sudo apt-get update
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