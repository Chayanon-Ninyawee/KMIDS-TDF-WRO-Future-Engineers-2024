cmake_minimum_required(VERSION 3.25.1)
project(TestV1 VERSION 0.0.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(RPLIDAR_SDK_PATH ${CMAKE_SOURCE_DIR}/external/rplidar_sdk/sdk)
include_directories(${RPLIDAR_SDK_PATH}/include)
include_directories(${RPLIDAR_SDK_PATH}/src)
file(GLOB_RECURSE RPLIDAR_SDK_SOURCES ${RPLIDAR_SDK_PATH}/src/*.cpp)
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*macOS.*")
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*win32.*")
add_library(rplidar_sdk STATIC ${RPLIDAR_SDK_SOURCES})

file(GLOB SOURCES "src/*.cpp")

add_executable(TestV1 ${SOURCES})

target_link_libraries(TestV1 wiringPi pthread rt rplidar_sdk SDL2 ${OpenCV_LIBS})

set_property(TARGET TestV1 PROPERTY CXX_STANDARD 17)