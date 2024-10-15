cmake_minimum_required(VERSION 3.25.1)
project(TestV1 VERSION 0.0.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(TestV1 src/main.cpp)

target_link_libraries(TestV1 wiringPi)

set_property(TARGET TestV1 PROPERTY CXX_STANDARD 17)