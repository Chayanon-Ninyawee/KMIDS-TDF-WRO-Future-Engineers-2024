#include <stdio.h>

#include "pico/stdlib.h"
#include "bno055Utils.h"

#include "debugPrint.h"

int main() {
    stdio_init_all();

    sleep_ms(2000);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);


    const uint sda_pin = 16;
    const uint scl_pin = 17;

    gpio_init(sda_pin);
    gpio_init(scl_pin);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    i2c_init(i2c_default, 400000);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    sleep_ms(5000);

    printf("Hello");

    bno055_t bno;
    bno055_initialize(&bno);
    sleep_ms(100);

    bno055_gyro_offset_t gyroOffset;
    bno055_accel_offset_t accelOffset;
    bno055_mag_offset_t magOffset;
    bno055_read_gyro_offset(&gyroOffset);
    bno055_read_accel_offset(&accelOffset);
    bno055_read_mag_offset(&magOffset);
    DEBUG_PRINT("gyro_offset - x: %d, y: %d, z: %d\n", gyroOffset.x, gyroOffset.y, gyroOffset.z);
    DEBUG_PRINT("accel_offset - x: %d, y: %d, z: %d, radius: %d\n", accelOffset.x, accelOffset.y, accelOffset.z, accelOffset.r);
    DEBUG_PRINT("mag_offset - x: %d, y: %d, z: %d, radius: %d\n", magOffset.x, magOffset.y, magOffset.z, magOffset.r);
    bno055_calibrate(&gyroOffset, &accelOffset, &magOffset);

    sleep_ms(1000);

    while (true) {
        bno055_accel_float_t accelData;
        bno055_convert_float_accel_xyz_msq(&accelData);
        printf("x: %3.2f,   y: %3.2f,   z: %3.2f\n", accelData.x, accelData.y, accelData.z);

        bno055_euler_float_t eulerAngles;
        bno055_convert_float_euler_hpr_deg(&eulerAngles);
        printf("h: %3.2f,   p: %3.2f,   r: %3.2f\n\n", eulerAngles.h, eulerAngles.p, eulerAngles.r);

        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(100);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(100);
    }

    return 0;
}