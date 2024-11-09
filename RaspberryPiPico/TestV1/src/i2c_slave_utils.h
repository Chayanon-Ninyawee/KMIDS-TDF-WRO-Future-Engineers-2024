#ifndef I2C_SLAVE_UTILS_H
#define I2C_SLAVE_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>

#include "bno055_utils.h"
#include "pico/i2c_slave.h"

// Command enums
enum command {
  DO_NOTHING = 0x00,
  RESTART = 0x01,
  BNO055_CALIB = 0x02,
  GET_BNO055_CALIB = 0x03,
  SEND_BNO055_CALIB = 0x04,
  GET_INFO = 0x05,
  SEND_INFO = 0x06
};

// RequestType enums
enum request_type_t {
  NOTHING = 0x00,
  BNO055_CALIB_DATA = 0x01,
  INFO_DATA = 0x02
};

// Structs for specific buffer types
struct bno055_calib_buffer {
  uint8_t buffer[22];  // For BNO055 calibration (22 bytes)
  uint8_t address;
};

struct recieved_info_buffer {
  uint8_t buffer[8];  // For general info (8 bytes)
  uint8_t address;
};

struct sending_info_buffer {
  uint8_t buffer[65536];  // Large buffer for logs
  uint16_t address;
};

// I2C state structure
struct I2C_slave_state {
  command recieved_command;
  bool is_command_set;
  request_type_t request_type;
  bno055_calib_buffer recieved_bno055_calib_bytes;
  recieved_info_buffer recieved_info_bytes;
  bno055_calib_buffer sending_bno055_calib_bytes;
  sending_info_buffer sending_info_bytes;

  void (*restart_callback)();
  void (*bno055_calib_callback)();
  void (*load_bno055_calib_callback)();
  void (*get_info_callback)();
};

extern I2C_slave_state i2c_slave_state;


void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);


/**
 * @brief Sets BNO055 offset data to be sent via I2C.
 * 
 * @param gyroOffset Pointer to the gyro offset structure.
 * @param accelOffset Pointer to the accel offset structure.
 * @param magOffset Pointer to the magnetometer offset structure.
 */
void set_bno055_offset_data(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset);

/**
 * @brief Sets the info data to be sent via I2C, including accelerometer, euler angles, and log.
 * 
 * @param accelData Pointer to the accelerometer data.
 * @param eulerAngles Pointer to the euler angle data.
 * @param logPtr Pointer to the log data string.
 */
void set_info_data(bno055_accel_float_t *accelData, bno055_euler_float_t *eulerAngles, char *logs);


/**
 * @brief Checks if the BNO055 offset data buffer contains valid data.
 * 
 * This function examines the BNO055 calibration buffer to determine if it 
 * has received valid data. If the buffer is entirely filled with the 
 * invalid marker (0xFF), the function returns false, indicating that no valid data is ready.
 * 
 * @return true if the buffer contains valid BNO055 offset data, false if the buffer 
 *         is entirely filled with the invalid marker (0xFF).
 */
bool is_bno055_offset_data_ready();

/**
 * @brief Retrieves BNO055 offset data if available, checking for buffer validity.
 * 
 * This function checks if the BNO055 calibration buffer contains valid data.
 * If the buffer is entirely filled with the invalid marker (0xFF), the function 
 * will return without modifying the output parameters.
 * 
 * @param outputGyroOffset Pointer to the gyro offset output structure. If the buffer is valid, 
 *                         this will be populated with gyro offset data.
 * @param outputAccelOffset Pointer to the accel offset output structure. If the buffer is valid, 
 *                          this will be populated with accelerometer offset data.
 * @param outputMagOffset Pointer to the magnetometer offset output structure. If the buffer is valid, 
 *                        this will be populated with magnetometer offset data.
 */
void get_bno055_offset_data(bno055_gyro_offset_t *outputGyroOffset, bno055_accel_offset_t *outputAccelOffset, bno055_mag_offset_t *outputMagOffset);

/**
 * @brief Checks if the motor and steering information buffer contains valid data.
 * 
 * This function examines the information buffer to determine if it has received valid data. 
 * If the buffer is entirely filled with the invalid marker (0xFF), the function returns 
 * false, indicating that no valid data is ready.
 * 
 * @return true if the buffer contains valid motor and steering information data, false if the buffer 
 *         is entirely filled with the invalid marker (0xFF).
 */
bool is_info_data_ready();

/**
 * @brief Retrieves motor and steering percentage information if available, checking for buffer validity.
 * 
 * This function verifies if the information buffer contains valid data.
 * If the buffer is entirely filled with the invalid marker (0xFF), the function 
 * will return without modifying the output parameters.
 * 
 * @param outputMotorPercent Pointer to the output motor percentage. If the buffer is valid, 
 *                           this will be populated with motor percentage data.
 * @param outputSteeringPercentage Pointer to the output steering percentage. If the buffer is valid, 
 *                                 this will be populated with steering percentage data.
 */
void get_info_data(float *outputMotorPercent, float *outputSteeringPercentage);



#endif  // I2C_SLAVE_UTILS_H
