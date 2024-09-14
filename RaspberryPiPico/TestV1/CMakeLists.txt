cmake_minimum_required(VERSION 3.13)

# pico_sdk_import.cmake is a single file copied from this SDK
include(pico-sdk/pico_sdk_init.cmake)

project(RaspiPicoTestV1 VERSION 0.0.0 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


add_subdirectory(libs/BNO055_driver)

file(GLOB SOURCES "src/*.cpp")

add_executable(RaspiPicoTestV1
    ${SOURCES}
)

target_link_libraries(RaspiPicoTestV1
    pico_stdlib
    hardware_i2c
    pico_bno055
)

pico_enable_stdio_usb(RaspiPicoTestV1 1)
pico_enable_stdio_uart(RaspiPicoTestV1 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(RaspiPicoTestV1)