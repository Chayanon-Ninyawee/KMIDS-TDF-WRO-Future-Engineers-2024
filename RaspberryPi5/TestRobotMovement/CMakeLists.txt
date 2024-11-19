cmake_minimum_required(VERSION 3.25.1)

project(TestRobotMovement VERSION 0.0.0 LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

include(CTest)
enable_testing()

file(GLOB SOURCES "src/*.cpp")

add_executable(TestRobotMovement ${SOURCES})

target_link_libraries(TestRobotMovement wiringPi SDL2)