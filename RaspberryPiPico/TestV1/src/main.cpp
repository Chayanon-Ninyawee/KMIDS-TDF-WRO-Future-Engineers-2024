#include <stdio.h>

#include "pico/stdlib.h"
#include "bno055Utils.h"

#include "debugPrint.h"

const uint sda0_pin = 16;
const uint scl0_pin = 17;

const uint sda1_pin = 18;
const uint scl1_pin = 19;


int main() {
    stdio_init_all();

    sleep_ms(2000);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Setup i2c0
    gpio_init(sda0_pin);
    gpio_init(scl0_pin);
    gpio_set_function(sda0_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl0_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda0_pin);
    gpio_pull_up(scl0_pin);
    i2c_init(i2c0, 400000);

    // Setup i2c1
    gpio_init(sda1_pin);
    gpio_init(scl1_pin);
    gpio_set_function(sda1_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl1_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda1_pin);
    gpio_pull_up(scl1_pin);
    i2c_init(i2c1, 400000);

    sleep_ms(100);

    bno055_t bno;
    bno055_initialize(&bno, i2c0);
    sleep_ms(100);

    bno055_gyro_offset_t gyroOffset;
    bno055_accel_offset_t accelOffset;
    bno055_mag_offset_t magOffset;
    bno055_calibrate(&gyroOffset, &accelOffset, &magOffset);
    sleep_ms(100);

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