cmake_minimum_required(VERSION 3.25.1)

project(TestRobotMovement VERSION 0.0.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAMERA REQUIRED libcamera)

find_library(LIBCAMERA_LIBRARY libcamera.so REQUIRED)
find_library(LIBCAMERA_BASE_LIBRARY libcamera-base.so REQUIRED)
message(STATUS ${CAMERA_INCLUDE_DIRS})
message(STATUS ${LIBCAMERA_LIBRARY})
message(STATUS ${LIBCAMERA_BASE_LIBRARY})

set(RPLIDAR_SDK_PATH ${CMAKE_SOURCE_DIR}/external/rplidar_sdk/sdk)
include_directories(${RPLIDAR_SDK_PATH}/include)
include_directories(${RPLIDAR_SDK_PATH}/src)
file(GLOB_RECURSE RPLIDAR_SDK_SOURCES ${RPLIDAR_SDK_PATH}/src/*.cpp)
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*macOS.*")
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*win32.*")
add_library(rplidar_sdk STATIC ${RPLIDAR_SDK_SOURCES})

include_directories(. "${CAMERA_INCLUDE_DIRS}")
set(LIBCAMERA_LIBRARIES "${LIBCAMERA_LIBRARY}" "${LIBCAMERA_BASE_LIBRARY}")

# Glob all .cpp files in the src directory
file(GLOB ALL_SOURCES "src/*.cpp")

# Glob only the main files
file(GLOB MAIN_FILES "src/main_*.cpp")

# Get the common sources by excluding the main files
set(COMMON_SOURCES ${ALL_SOURCES})
list(REMOVE_ITEM COMMON_SOURCES ${MAIN_FILES})

add_executable(TestV1 src/main_testv1.cpp ${COMMON_SOURCES})
target_link_libraries(TestV1 wiringPi pthread rt SDL2 rplidar_sdk "${LIBCAMERA_LIBRARIES}" ${OpenCV_LIBS})

add_executable(LoadLidarFile src/main_load_lidar_file.cpp ${COMMON_SOURCES})
target_link_libraries(LoadLidarFile wiringPi pthread rplidar_sdk "${LIBCAMERA_LIBRARIES}" ${OpenCV_LIBS})

add_executable(CamTestV1 src/main_cam_testv1.cpp ${COMMON_SOURCES})
target_link_libraries(CamTestV1 wiringPi rplidar_sdk "${LIBCAMERA_LIBRARIES}" ${OpenCV_LIBS})