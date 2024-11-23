cmake_minimum_required(VERSION 3.25.1)

project(TestRobotMovement VERSION 0.0.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")


include(CTest)
enable_testing()


find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )


find_package(PkgConfig REQUIRED)
pkg_check_modules(CAMERA libcamera)

if (CAMERA_FOUND)
    find_library(LIBCAMERA_LIBRARY libcamera.so)
    find_library(LIBCAMERA_BASE_LIBRARY libcamera-base.so)

    if (LIBCAMERA_LIBRARY AND LIBCAMERA_BASE_LIBRARY)
        set(LIBCAMERA_FOUND TRUE)
        include_directories(${CAMERA_INCLUDE_DIRS})
        set(LIBCAMERA_LIBRARIES "${LIBCAMERA_LIBRARY}" "${LIBCAMERA_BASE_LIBRARY}")
        message(STATUS "libCamera found: ${CAMERA_INCLUDE_DIRS}")
        message(STATUS "libCamera libraries: ${LIBCAMERA_LIBRARIES}")
    else()
        set(LIBCAMERA_FOUND FALSE)
        message(WARNING "libCamera libraries not found, skipping executables requiring libCamera.")
    endif()
else()
    set(LIBCAMERA_FOUND FALSE)
    message(WARNING "libCamera module not found, skipping executables requiring libCamera.")
endif()


find_library(WIRINGPI_LIBRARY wiringPi)
if (WIRINGPI_LIBRARY)
    set(WIRINGPI_FOUND TRUE)
    message(STATUS "wiringPi library found: ${WIRINGPI_LIBRARY}")
else()
    set(WIRINGPI_FOUND FALSE)
    message(WARNING "wiringPi library not found, skipping executables requiring wiringPi.")
endif()


set(RPLIDAR_SDK_PATH ${CMAKE_SOURCE_DIR}/external/rplidar_sdk/sdk)
include_directories(${RPLIDAR_SDK_PATH}/include)
include_directories(${RPLIDAR_SDK_PATH}/src)
file(GLOB_RECURSE RPLIDAR_SDK_SOURCES ${RPLIDAR_SDK_PATH}/src/*.cpp)
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*macOS.*")
list(FILTER RPLIDAR_SDK_SOURCES EXCLUDE REGEX ".*win32.*")
add_library(rplidar_sdk STATIC ${RPLIDAR_SDK_SOURCES})



add_library(DataSaverUtils STATIC
    src/utils/dataSaver.cpp
)
target_include_directories(DataSaverUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DataSaverUtils ${OpenCV_LIBS})


add_library(DirectionUtils STATIC
    src/utils/direction.cpp
)
target_include_directories(DirectionUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


if (WIRINGPI_FOUND)
    add_library(I2CMasterUtils STATIC
        src/utils/i2c_master.cpp
    )
    target_include_directories(I2CMasterUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(I2CMasterUtils wiringPi)
else()
    message(WARNING "Skipping I2CMasterUtils as wiringPi is not available.")
endif()


add_library(ImageProcessorUtils STATIC
    src/utils/imageProcessor.cpp
)
target_include_directories(ImageProcessorUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ImageProcessorUtils ${OpenCV_LIBS})


if (LIBCAMERA_FOUND)
    add_library(LibCameraUtils STATIC
        src/utils/libCamera.cpp
    )
    target_include_directories(LibCameraUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(LibCameraUtils ${LIBCAMERA_LIBRARIES})
else()
    message(WARNING "Skipping CamTestV1 as libCamera is not available.")
endif()


add_library(LidarControllerUtils STATIC
    src/utils/lidarController.cpp
)
target_include_directories(LidarControllerUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LidarControllerUtils rplidar_sdk)


add_library(LidarDataProcessorUtils STATIC
    src/utils/lidarDataProcessor.cpp
    src/utils/direction.cpp
)
target_include_directories(LidarDataProcessorUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LidarDataProcessorUtils LidarControllerUtils ${OpenCV_LIBS})




# Helper function to verify libraries are available before linking
function(verify_and_add_executable target_name source_files dependencies)
    foreach(dep ${dependencies})
        # Check if the dependency is a valid library or target
        if (NOT TARGET ${dep})
            find_library(LIB_PATH ${dep})
            if (NOT LIB_PATH)
                message(WARNING "Skipping ${target_name} because dependency ${dep} is missing.")
                return()
            endif()
        endif()
    endforeach()
    add_executable(${target_name} ${source_files})
    target_link_libraries(${target_name} ${dependencies})
endfunction()

# OpenChallenge executable
if (LIBCAMERA_FOUND)
    verify_and_add_executable(OpenChallenge 
        "src/main_openChallenge.cpp;src/challenges/openChallenge.cpp" 
        "I2CMasterUtils;LibCameraUtils;LidarControllerUtils;LidarDataProcessorUtils;DataSaverUtils"
    )
else()
    message(WARNING "Skipping OpenChallenge as libCamera is not available.")
endif()

# LidarTestV1 executable
if (LIBCAMERA_FOUND)
    verify_and_add_executable(LidarTestV1 
        "src/main_lidar_testv1.cpp" 
        "I2CMasterUtils;LibCameraUtils;LidarControllerUtils;LidarDataProcessorUtils;ImageProcessorUtils;DataSaverUtils"
    )
else()
    message(WARNING "Skipping LidarTestV1 as libCamera is not available.")
endif()

# CamTestV1 executable
if (LIBCAMERA_FOUND)
    verify_and_add_executable(CamTestV1 
        "src/main_cam_testv1.cpp" 
        "${OpenCV_LIBS};LibCameraUtils"
    )
else()
    message(WARNING "Skipping CamTestV1 as libCamera is not available.")
endif()

# MoveTestV1 executable
verify_and_add_executable(MoveTestV1 
    "src/main_move_testv1.cpp" 
    "SDL2;I2CMasterUtils"
)

# LoadLidarFile executable
verify_and_add_executable(LoadLidarFile 
    "src/main_load_lidar_file.cpp" 
    "LidarControllerUtils;LidarDataProcessorUtils;ImageProcessorUtils;DataSaverUtils"
)
