#include <stdio.h>

#include "bno055_utils.h"
#include "debug_print.h"
#include "i2c_slave_utils.h"
#include "pico/stdlib.h"
#include "pwm_utils.h"

const uint I2C0_SDA_PIN = 16;
const uint I2C0_SCL_PIN = 17;
const uint I2C0_BAUDRATE = 400000;  // 400 kHz

const uint I2C1_SDA_PIN = 18;
const uint I2C1_SCL_PIN = 19;
const uint I2C1_BAUDRATE = 400000;  // 400 kHz
const uint I2C1_SLAVE_ADDR = 0x39;

const uint SERVO_PIN = 22;
const uint SERVO_MIN_ANGLE = 50;
const uint SERVO_MAX_ANGLE = 180;

const uint MOTOR_A_PIN = 4;
const uint MOTOR_B_PIN = 5;


char logs[] = "Test";


int8_t is_bno055_calib_exist = -1;


float motorPercent = 0.0f;
float steeringPercent = 0.0f;


void handle_restart() {
  
}

void handle_bno055_calib() {
  is_bno055_calib_exist = 0;
}

void handle_load_bno055_calib() {
  is_bno055_calib_exist = 1;
}

void handle_get_info() {
  get_info_data(&motorPercent, &steeringPercent);
}

int main() {
  std::fill_n(i2c_slave_state.sending_bno055_calib_bytes.buffer, sizeof(i2c_slave_state.sending_bno055_calib_bytes.buffer), 0xFF);
  std::fill_n(i2c_slave_state.sending_info_bytes.buffer, sizeof(i2c_slave_state.sending_info_bytes.buffer), 0xFF);

  i2c_slave_state.restart_callback = handle_restart;
  i2c_slave_state.bno055_calib_callback = handle_bno055_calib;
  i2c_slave_state.load_bno055_calib_callback = handle_load_bno055_calib;
  i2c_slave_state.get_info_callback = handle_get_info;


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

  while (is_bno055_calib_exist == -1) {
    sleep_ms(10);
  }

  bno055_gyro_offset_t gyroOffset;
  bno055_accel_offset_t accelOffset;
  bno055_mag_offset_t magOffset;

  if (is_bno055_calib_exist == 1) {
    get_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);
    bno055_load_offset(&gyroOffset, &accelOffset, &magOffset);
    sleep_ms(100);
  }

  bno055_calibrate(&gyroOffset, &accelOffset, &magOffset);
  sleep_ms(100);
  set_bno055_offset_data(&gyroOffset, &accelOffset, &magOffset);

  while (true) {
    bno055_accel_float_t accelData;
    bno055_convert_float_accel_xyz_msq(&accelData);
    DEBUG_PRINT("x: %3.2f,   y: %3.2f,   z: %3.2f\n", accelData.x, accelData.y, accelData.z);

    bno055_euler_float_t eulerAngles;
    bno055_convert_float_euler_hpr_deg(&eulerAngles);
    DEBUG_PRINT("h: %3.2f,   p: %3.2f,   r: %3.2f\n\n", eulerAngles.h, eulerAngles.p, eulerAngles.r);

    set_info_data(&accelData, &eulerAngles, logs);

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