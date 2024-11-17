#include "i2c_slave_utils.h"

using namespace i2c_slave_mem_addr;

void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE:  // master has written some data
      if (!context.mem_address_written) {
        // writes always start with the memory address
        context.mem_address = i2c_read_byte_raw(i2c);
        context.mem_address_written = true;
      } else {
        // save into memory
        context.mem[context.mem_address] = i2c_read_byte_raw(i2c);
        context.mem_address++;
      }
      break;
    case I2C_SLAVE_REQUEST:  // master is requesting data
      // load from memory
      i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
      context.mem_address++;
      break;
    case I2C_SLAVE_FINISH:  // master has signalled Stop / Restart
      context.mem_address_written = false;
      break;
    default:
      break;
  }
}


uint8_t test() {
  return context.mem_address;
}

void context_init() {
  std::fill_n(context.mem, sizeof(context.mem), 0);
  context.mem_address = 0;
  context.mem_address_written = false;
}

uint8_t get_command() {
  return context.mem[COMMAND_ADDR];
}

void set_command(uint8_t command) {
  context.mem[COMMAND_ADDR] = command;
}


void set_is_running(bool is_running) {
  auto status_byte = context.mem[STATUS_ADDR];
  if (is_running) {
    status_byte |= (1 << 0);
  } else {
    status_byte &= ~(1 << 0);
  }
  context.mem[STATUS_ADDR] = status_byte;
}

bool get_is_running() {
  return context.mem[STATUS_ADDR] & (1 << 0);
}


void set_is_calib_offset_ready(bool is_calib_offset_ready) {
  auto status_byte = context.mem[STATUS_ADDR];
  if (is_calib_offset_ready) {
      status_byte |= (1 << 1);
  } else {
      status_byte &= ~(1 << 1);
  }
  context.mem[STATUS_ADDR] = status_byte;
}

bool get_is_calib_offset_ready() {
  return context.mem[STATUS_ADDR] & (1 << 1);
}


void set_is_bno055_info_ready(bool is_bno055_info_ready) {
  auto status_byte = context.mem[STATUS_ADDR];
  if (is_bno055_info_ready) {
      status_byte |= (1 << 2);
  } else {
      status_byte &= ~(1 << 2);
  }
  context.mem[STATUS_ADDR] = status_byte;
}

bool get_is_bno055_info_ready() {
  return context.mem[STATUS_ADDR] & (1 << 2);
}


bool is_bno055_offset_data_ready() {
  for (size_t i = BNO055_CALIB_ADDR; i < BNO055_CALIB_ADDR + BNO055_CALIB_SIZE; ++i) {
    if (context.mem[i] != 0xFF) {
      return true;
    }
  }
  return false;
}

void get_bno055_offset_data(bno055_gyro_offset_t *outputGyroOffset, bno055_accel_offset_t *outputAccelOffset, bno055_mag_offset_t *outputMagOffset) {
  if (!is_bno055_offset_data_ready()) return;

  uint8_t *buffer_ptr = &context.mem[BNO055_CALIB_ADDR];
  memcpy(outputGyroOffset, buffer_ptr, GYRO_OFFSET_SIZE);
  memcpy(outputAccelOffset, buffer_ptr + GYRO_OFFSET_SIZE, ACCEL_OFFSET_SIZE);
  memcpy(outputMagOffset, buffer_ptr + GYRO_OFFSET_SIZE + ACCEL_OFFSET_SIZE, MAG_OFFSET_SIZE);
}

void set_bno055_offset_data(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset) {
  uint8_t *buffer_ptr = &context.mem[BNO055_CALIB_ADDR];
  memcpy(buffer_ptr, gyroOffset, GYRO_OFFSET_SIZE);
  memcpy(buffer_ptr + GYRO_OFFSET_SIZE, accelOffset, ACCEL_OFFSET_SIZE);
  memcpy(buffer_ptr + GYRO_OFFSET_SIZE + ACCEL_OFFSET_SIZE, magOffset, MAG_OFFSET_SIZE);
}


bool is_movement_info_data_ready() {
  for (size_t i = MOVEMENT_INFO_ADDR; i < MOVEMENT_INFO_ADDR + MOVEMENT_INFO_SIZE; ++i) {
    if (context.mem[i] != 0xFF) {
      return true;
    }
  }
  return false;
}

void get_movement_info_data(float *outputMotorPercent, float *outputSteeringPercentage) {
  if (!is_movement_info_data_ready()) return;

  uint8_t *buffer_ptr = &context.mem[MOVEMENT_INFO_ADDR];
  memcpy(outputMotorPercent, buffer_ptr, MOTOR_PERCENT_SIZE);
  memcpy(outputSteeringPercentage, buffer_ptr + MOTOR_PERCENT_SIZE, STEERING_PERCENT_SIZE);
}

void set_bno055_info_data(bno055_accel_float_t *accelData, bno055_euler_float_t *eulerAngles, char *logs) {
  bno055_euler_float_t angleData = *eulerAngles;
  bno055_convert_float_euler_hpr_deg(&angleData);

  uint8_t *buffer_ptr = &context.mem[BNO055_INFO_ADDR];
  memcpy(buffer_ptr, accelData, ACCEL_DATA_SIZE);
  memcpy(buffer_ptr + ACCEL_DATA_SIZE, &angleData, EULER_ANGLE_SIZE);

  const size_t log_offset = ACCEL_DATA_SIZE + EULER_ANGLE_SIZE;
  const size_t max_log_size = LOG_SIZE;

  size_t log_size = strnlen(logs, max_log_size);
  memcpy(buffer_ptr + log_offset, logs, log_size);

  // Set the next 5 bytes as the unique ending marker (0xFF)
  memset(buffer_ptr + log_offset + log_size, 0xFF, 5);
}