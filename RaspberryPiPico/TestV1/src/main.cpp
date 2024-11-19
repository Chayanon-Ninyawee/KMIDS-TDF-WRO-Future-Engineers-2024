#include <stdio.h>

#include "bno055_utils.h"
#include "debug_print.h"
#include "i2c_slave_utils.h"
#include "pico/stdlib.h"
#include "pwm_utils.h"

const uint I2C0_SDA_PIN = 16;
const uint I2C0_SCL_PIN = 17;
const uint I2C0_BAUDRATE = 400000;  // 400 kHz

const uint I2C1_SDA_PIN = 26;
const uint I2C1_SCL_PIN = 27;
const uint I2C1_BAUDRATE = 400000;  // 400 kHz
const uint I2C1_SLAVE_ADDR = 0x39;

const uint SERVO_PIN = 22;
const uint SERVO_MIN_ANGLE = 50;
const uint SERVO_MAX_ANGLE = 180;

const uint MOTOR_A_PIN = 4;
const uint MOTOR_B_PIN = 5;

int main() {
  stdio_init_all();

  sleep_ms(2000);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  setup_servo(SERVO_PIN, (SERVO_MAX_ANGLE + SERVO_MIN_ANGLE) / 2.0f);

  setup_L9110S_motor_driver(MOTOR_A_PIN, MOTOR_B_PIN);

  // Setup i2c0
  gpio_init(I2C0_SDA_PIN);
  gpio_init(I2C0_SCL_PIN);
  gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C0_SDA_PIN);
  gpio_pull_up(I2C0_SCL_PIN);
  i2c_init(i2c0, I2C0_BAUDRATE);

  bno055_t bno;
  bno055_initialize(&bno, i2c0);

  // Setup i2c1
  gpio_init(I2C1_SDA_PIN);
  gpio_init(I2C1_SCL_PIN);
  gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C1_SDA_PIN);
  gpio_pull_up(I2C1_SCL_PIN);
  i2c_init(i2c1, I2C1_BAUDRATE);
  i2c_slave_init(i2c1, I2C1_SLAVE_ADDR, &i2c_slave_handler);

  start:
  i2c_slave_context_init();

  set_is_running(true);

  uint8_t currentCommand = get_command();
  while (not (
    currentCommand == Command::CALIB_NO_OFFSET or
    currentCommand == Command::CALIB_WITH_OFFSET or
    currentCommand == Command::SKIP_CALIB
  )) {
    if (currentCommand == Command::RESTART) goto start; // Restart main
    DEBUG_PRINT("Waiting for calibration command...\n");
    sleep_ms(100);
    currentCommand = get_command();
  }

  bno055_gyro_offset_t gyroOffset;
  bno055_accel_offset_t accelOffset;
  bno055_mag_offset_t magOffset;

  switch (currentCommand) {
  case Command::CALIB_WITH_OFFSET:
    get_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);
    bno055_load_offset(&gyroOffset, &accelOffset, &magOffset);
    sleep_ms(100);
    // Fall-through intended to execute calibration steps
  case Command::CALIB_NO_OFFSET:
    bno055_calibrate(&gyroOffset, &accelOffset, &magOffset);
    set_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);
    break;
  case Command::SKIP_CALIB:
    break;
  default:
    // Send logs
    return -1;
  }
  set_is_calib_offset_ready(true);
  sleep_ms(100);


  while (true) {
    uint8_t currentCommand = get_command();
    if (currentCommand == Command::RESTART) goto start; // Restart main

    bno055_accel_float_t accelData;
    bno055_convert_float_accel_xyz_msq(&accelData);
    // DEBUG_PRINT("x: %3.2f,   y: %3.2f,   z: %3.2f\n", accelData.x, accelData.y, accelData.z);

    bno055_euler_float_t eulerAngles;
    bno055_convert_float_euler_hpr_deg(&eulerAngles);
    // DEBUG_PRINT("h: %3.2f,   p: %3.2f,   r: %3.2f\n\n", eulerAngles.h, eulerAngles.p, eulerAngles.r);

    set_bno055_info_data(&accelData, &eulerAngles);
    set_is_bno055_info_ready(true);


    float motorPercent = 0.0f;
    float steeringPercent = 0.0f;

    get_movement_info_data(&motorPercent, &steeringPercent);
    // DEBUG_PRINT("%0.2f, %0.2f\n", motorPercent, steeringPercent);

    set_L9110S_motor_speed(MOTOR_A_PIN, MOTOR_B_PIN, motorPercent);
    set_servo_angle(SERVO_PIN, SERVO_MIN_ANGLE + (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) * ((steeringPercent + 1.0) / 2.0f));
  }

  return 0;
}