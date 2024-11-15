add_library(pico_bno055 
    INTERFACE
)

target_sources(pico_bno055
    INTERFACE
        bno055.c bno055.h
)

target_include_directories(pico_bno055
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(pico_bno055 
    INTERFACE
        hardware_gpio
        hardware_i2c
)