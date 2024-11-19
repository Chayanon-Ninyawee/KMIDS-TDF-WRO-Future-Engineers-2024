#ifndef I2C_SLAVE_UTILS_H
#define I2C_SLAVE_UTILS_H

#include <string.h>
#include <algorithm>

#include "bno055_utils.h"
#include "pico/i2c_slave.h"

namespace i2c_slave_mem_addr {
  const size_t MEM_SIZE = 256;

  const size_t COMMAND_SIZE = 1; // This is 1 byte

  const size_t STATUS_SIZE = 1; // This is 1 byte

  const size_t GYRO_OFFSET_SIZE = sizeof(bno055_gyro_offset_t);
  const size_t ACCEL_OFFSET_SIZE = sizeof(bno055_accel_offset_t);
  const size_t MAG_OFFSET_SIZE = sizeof(bno055_mag_offset_t);
  const size_t BNO055_CALIB_SIZE = (GYRO_OFFSET_SIZE + ACCEL_OFFSET_SIZE + MAG_OFFSET_SIZE);

  const size_t ACCEL_DATA_SIZE = sizeof(bno055_accel_float_t);
  const size_t EULER_ANGLE_SIZE = sizeof(bno055_euler_float_t);
  const size_t BNO055_INFO_SIZE = (ACCEL_DATA_SIZE + EULER_ANGLE_SIZE);

  const size_t MOTOR_PERCENT_SIZE = sizeof(float);
  const size_t STEERING_PERCENT_SIZE = sizeof(float);
  const size_t MOVEMENT_INFO_SIZE = (MOTOR_PERCENT_SIZE + STEERING_PERCENT_SIZE);

  const size_t LOG_SIZE = 1;
  const size_t LOGS_BUFFER_SIZE = 256;

  const size_t COMMAND_ADDR = 0;
  const size_t STATUS_ADDR = (COMMAND_ADDR + COMMAND_SIZE);
  const size_t BNO055_CALIB_ADDR = (STATUS_ADDR + STATUS_SIZE);
  const size_t BNO055_INFO_ADDR = (BNO055_CALIB_ADDR + BNO055_CALIB_SIZE);
  const size_t MOVEMENT_INFO_ADDR = (BNO055_INFO_ADDR + BNO055_INFO_SIZE);
  const size_t LOG_ADDR = (MOVEMENT_INFO_ADDR + MOVEMENT_INFO_SIZE);


  // Check if total memory allocation fits within the available memory
  static_assert(LOG_ADDR + LOG_SIZE <= MEM_SIZE, "Memory allocation exceeds buffer size");
}


enum Command : uint8_t {
  NO_COMMAND = 0x00,
  RESTART = 0x01,
  CALIB_NO_OFFSET = 0x02,
  CALIB_WITH_OFFSET = 0x03,
  SKIP_CALIB = 0x04
};

struct context_t {
  uint8_t mem[i2c_slave_mem_addr::MEM_SIZE];
  uint8_t mem_address;
  bool mem_address_written;
  uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE];
  uint16_t logs_start;
  uint16_t logs_count;
  uint16_t logs_reading_address;
  bool logs_read;
};

context_t context;

void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

void i2c_slave_context_init();

/**
 * @brief Retrieve the current command from memory.
 * @return The current command as a Command enum.
 */
uint8_t get_command();

/**
 * @brief Set a command in memory.
 * @param command The command to set.
 */
void set_command(uint8_t command);

/**
 * @brief Set the running status in memory.
 * @param is_running True if running, false otherwise.
 */
void set_is_running(bool is_running);

/**
 * @brief Retrieve the running status from memory.
 * @return True if running, false otherwise.
 */
bool get_is_running();

/**
 * @brief Set the calibration offset readiness status in memory.
 * @param is_calib_offset_ready True if calibration offset is ready, false otherwise.
 */
void set_is_calib_offset_ready(bool is_calib_offset_ready);

/**
 * @brief Retrieve the calibration offset readiness status from memory.
 * @return True if calibration offset is ready, false otherwise.
 */
bool get_is_calib_offset_ready();

/**
 * @brief Set the BNO055 information readiness status in memory.
 * @param is_bno055_info_ready True if BNO055 information is ready, false otherwise.
 */
void set_is_bno055_info_ready(bool is_bno055_info_ready);

/**
 * @brief Retrieve the BNO055 information readiness status from memory.
 * @return True if BNO055 information is ready, false otherwise.
 */
bool get_is_bno055_info_ready();

/**
 * @brief Check if the BNO055 offset data is ready.
 * @return True if offset data is ready, otherwise false.
 */
bool is_bno055_offset_data_ready();

/**
 * @brief Retrieve BNO055 offset data if available.
 * @param outputGyroOffset Pointer to store gyro offset data.
 * @param outputAccelOffset Pointer to store accelerometer offset data.
 * @param outputMagOffset Pointer to store magnetometer offset data.
 */
void get_bno055_offset_data(bno055_gyro_offset_t *outputGyroOffset, bno055_accel_offset_t *outputAccelOffset, bno055_mag_offset_t *outputMagOffset);

/**
 * @brief Set BNO055 offset data in memory.
 * @param gyroOffset Pointer to gyro offset data.
 * @param accelOffset Pointer to accelerometer offset data.
 * @param magOffset Pointer to magnetometer offset data.
 */
void set_bno055_offset_data(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset);

/**
 * @brief Check if movement information data is ready.
 * @return True if movement data is ready, otherwise false.
 */
bool is_movement_info_data_ready();

/**
 * @brief Retrieve movement information data if available.
 * @param outputMotorPercent Pointer to store motor percentage.
 * @param outputSteeringPercentage Pointer to store steering percentage.
 */
void get_movement_info_data(float *outputMotorPercent, float *outputSteeringPercentage);

/**
 * @brief Set BNO055 information data and logs in memory.
 * @param accelData Pointer to accelerometer data.
 * @param eulerAngles Pointer to Euler angle data.
 * @param logs Pointer to log data string.
 */
void set_bno055_info_data(bno055_accel_float_t *accelData, bno055_euler_float_t *eulerAngles);


/**
 * @brief Appends new logs to the existing logs buffer with rolling behavior.
 *
 * This function adds the provided logs to the current logs buffer. If there 
 * is not enough space to append the new logs, the oldest logs in the buffer 
 * are overwritten to make room. The buffer operates as a rolling log, ensuring 
 * that the most recent logs are always retained.
 *
 * A unique ending marker (0xFF) is added after the new logs, provided there 
 * is space for it. If the buffer becomes full, the marker wraps around.
 *
 * @param logs A pointer to the string buffer containing the logs to append.
 *             The logs do not need to be null-terminated as their length is 
 *             explicitly provided.
 * @param len The length of the logs to append. If `len` exceeds the available 
 *            buffer space, the logs will be truncated to fit.
 * 
 * @note The buffer rolls over and overwrites the oldest logs when it is full.
 * @note The writing address wraps around to the beginning of the buffer 
 *       when reaching the end.
 */
void append_logs(const char *logs, size_t len);

uint8_t* get_logs();

#endif  // I2C_SLAVE_UTILS_H
