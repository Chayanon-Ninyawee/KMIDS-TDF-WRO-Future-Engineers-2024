#include "i2c_slave_utils.h"

using namespace i2c_slave_mem_addr;

void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE:  // master has written some data
      if (!context.mem_address_written) {
        // writes always start with the memory address
        context.mem_address = i2c_read_byte_raw(i2c);
        context.mem_address_written = true;

        if (context.mem_address == LOG_ADDR) context.logs_reading_address = 0;
      } else {
        // save into memory
        context.mem[context.mem_address] = i2c_read_byte_raw(i2c);
        context.mem_address++;
      }
      break;
    case I2C_SLAVE_REQUEST:  // master is requesting data
      if (context.mem_address == LOG_ADDR) {
        // load from logs
        if (context.logs_reading_address < context.logs_count) {
          i2c_write_byte_raw(i2c, context.logs[(context.logs_start + context.logs_reading_address) % LOGS_BUFFER_SIZE]);
          context.logs_reading_address++;
        } else {
          // Send 0xFF if logs are exhausted
          i2c_write_byte_raw(i2c, 0xFF);
        }
        context.logs_read = true;
      } else {
        // load from memory
        i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
        context.mem_address++;
      }
      break;
    case I2C_SLAVE_FINISH:  // master has signalled Stop / Restart
      if (context.logs_read) {
        std::fill_n(context.logs, sizeof(context.logs), 0);
        context.logs[0] = 0xFF;
        context.logs_start = 0;
        context.logs_count = 0;
        context.logs_reading_address = 0;
        context.logs_read = false;
      }
      context.mem_address_written = false;
      break;
    default:
      break;
  }
}


void i2c_slave_context_init() {
  std::fill_n(context.mem, sizeof(context.mem), 0);
  context.mem_address = 0;
  context.mem_address_written = false;

  std::fill_n(context.logs, sizeof(context.logs), 0);
  context.logs[0] = 0xFF;
  context.logs_start = 0;
  context.logs_count = 0;
  context.logs_reading_address = 0;
  context.logs_read = false;
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

void set_bno055_info_data(bno055_accel_float_t *accelData, bno055_euler_float_t *eulerAngles) {
  bno055_euler_float_t angleData = *eulerAngles;
  bno055_convert_float_euler_hpr_deg(&angleData);

  uint8_t *buffer_ptr = &context.mem[BNO055_INFO_ADDR];
  memcpy(buffer_ptr, accelData, ACCEL_DATA_SIZE);
  memcpy(buffer_ptr + ACCEL_DATA_SIZE, &angleData, EULER_ANGLE_SIZE);
}


void append_logs(const char *logs, size_t len) {
    const size_t max_log_size = LOGS_BUFFER_SIZE;

    // If the new data is larger than the buffer size, only keep the last `max_log_size` bytes
    if (len > max_log_size) {
        logs += len - max_log_size;
        len = max_log_size;
    }

    // Calculate space needed and adjust start pointer if necessary
    size_t space_needed = len;
    size_t available_space = max_log_size - context.logs_count;

    if (space_needed > available_space) {
        // Remove oldest logs to make space for the new ones
        context.logs_start = (context.logs_start + (space_needed - available_space)) % max_log_size;
        context.logs_count -= (space_needed - available_space);
    }

    // Write the new logs into the buffer
    size_t write_pos = (context.logs_start + context.logs_count) % max_log_size;

    if (write_pos + len <= max_log_size) {
        // Case 1: Logs fit without wrapping
        memcpy(&context.logs[write_pos], logs, len);
    } else {
        // Case 2: Logs wrap around
        size_t first_chunk = max_log_size - write_pos;
        memcpy(&context.logs[write_pos], logs, first_chunk);
        memcpy(&context.logs[0], logs + first_chunk, len - first_chunk);
    }

    // Update the count and ending marker
    context.logs_count += len;
    if (context.logs_count > max_log_size) {
        context.logs_count = max_log_size; // Prevent overflow
    }

    // Set the unique ending marker
    size_t end_pos = (context.logs_start + context.logs_count) % max_log_size;
    if (end_pos != context.logs_start) {
      context.logs[end_pos] = 0xFF;
    }
}

uint8_t* get_logs() {
  return context.logs;
}