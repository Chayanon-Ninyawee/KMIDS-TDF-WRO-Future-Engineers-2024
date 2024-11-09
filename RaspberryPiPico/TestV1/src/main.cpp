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


char logs[] = "Test";

float motorPercent = 0.0f;
float steeringPercent = 0.0f;

int main() {
  context_init();

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


  set_is_running(true);
  while (not (get_command() == Command::CALIB_NO_OFFSET or get_command() == Command::CALIB_WITH_OFFSET)) {
    printf("%d, %d, %d\n", (int)test(), get_command(), not (get_command() == Command::CALIB_NO_OFFSET or get_command() == Command::CALIB_WITH_OFFSET));
    sleep_ms(100);
  }

  bno055_gyro_offset_t gyroOffset;
  bno055_accel_offset_t accelOffset;
  bno055_mag_offset_t magOffset;

  if (get_command() == Command::CALIB_WITH_OFFSET) {
    get_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);
    bno055_load_offset(&gyroOffset, &accelOffset, &magOffset);
    sleep_ms(100);
  }

  bno055_calibrate(&gyroOffset, &accelOffset, &magOffset);
  set_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);
  set_is_calib_offset_ready(true);
  sleep_ms(100);

  while (true) {
    bno055_accel_float_t accelData;
    bno055_convert_float_accel_xyz_msq(&accelData);
    // DEBUG_PRINT("x: %3.2f,   y: %3.2f,   z: %3.2f\n", accelData.x, accelData.y, accelData.z);

    bno055_euler_float_t eulerAngles;
    bno055_convert_float_euler_hpr_deg(&eulerAngles);
    // DEBUG_PRINT("h: %3.2f,   p: %3.2f,   r: %3.2f\n\n", eulerAngles.h, eulerAngles.p, eulerAngles.r);

    set_bno055_info_data(&accelData, &eulerAngles, logs);
    set_is_bno055_info_ready(true);

    // gpio_put(PICO_DEFAULT_LED_PIN, true);
    // sleep_ms(100);
    // gpio_put(PICO_DEFAULT_LED_PIN, false);
    // sleep_ms(100);

    // gpio_put(PICO_DEFAULT_LED_PIN, false);
    // set_L9110S_motor_speed(MOTOR_A_PIN, MOTOR_B_PIN, 0.0f);
    // sleep_ms(1000);

    // gpio_put(PICO_DEFAULT_LED_PIN, true);
    // set_L9110S_motor_speed(MOTOR_A_PIN, MOTOR_B_PIN, 1.0f);
    // sleep_ms(3000);
    // set_L9110S_motor_speed(MOTOR_A_PIN, MOTOR_B_PIN, 0.0f);
    // sleep_ms(100);
    // set_L9110S_motor_speed(MOTOR_A_PIN, MOTOR_B_PIN, -1.0f);
    // sleep_ms(3000);
  }

  return 0;
}