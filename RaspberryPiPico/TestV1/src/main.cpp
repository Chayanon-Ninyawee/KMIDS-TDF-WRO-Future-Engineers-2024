#include <stdio.h>

#include "pico/i2c_slave.h"
#include "pico/stdlib.h"
#include "bno055_utils.h"

#include "debug_print.h"

static const uint I2C0_SDA_PIN = 16;
static const uint I2C0_SCL_PIN = 17;
static const uint I2C0_BAUDRATE = 400000; // 400 kHz

static const uint I2C1_SDA_PIN = 18;
static const uint I2C1_SCL_PIN = 19;
static const uint I2C1_BAUDRATE = 400000; // 400 kHz
static const uint I2C1_SLAVE_ADDR = 0x39;


int main() {
    stdio_init_all();

    sleep_ms(2000);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Setup i2c0
    gpio_init(I2C0_SDA_PIN);
    gpio_init(I2C0_SCL_PIN);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);
    i2c_init(i2c0, I2C0_BAUDRATE);

    // Setup i2c1
    gpio_init(I2C1_SDA_PIN);
    gpio_init(I2C1_SCL_PIN);
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    i2c_init(i2c1, I2C1_BAUDRATE);
    i2c_slave_init(i2c1, I2C1_SLAVE_ADDR, &i2c_slave_handler);

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