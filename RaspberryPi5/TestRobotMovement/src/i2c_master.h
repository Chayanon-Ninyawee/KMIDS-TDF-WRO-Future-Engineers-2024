#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stddef.h>
#include <stdint.h>

namespace i2c_slave_mem_addr {
  const size_t MEM_SIZE = 256;

  const size_t COMMAND_SIZE = 1; // This is 1 byte

  const size_t STATUS_SIZE = 1; // This is 1 byte

  const size_t GYRO_OFFSET_SIZE = 6;
  const size_t ACCEL_OFFSET_SIZE = 8;
  const size_t MAG_OFFSET_SIZE = 8;
  const size_t BNO055_CALIB_SIZE = (GYRO_OFFSET_SIZE + ACCEL_OFFSET_SIZE + MAG_OFFSET_SIZE);

  const size_t ACCEL_DATA_SIZE = 12;
  const size_t EULER_ANGLE_SIZE = 12;
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

// Function declarations
int i2c_master_init(uint8_t slave_address);
void i2c_master_send_command(int fd, uint8_t command);
void i2c_master_send_data(int fd, uint8_t reg, uint8_t *data, uint8_t len);
void i2c_master_read_data(int fd, uint8_t reg, uint8_t *data, uint8_t len);
void i2c_master_read_logs(int fd, uint8_t *logs);
void i2c_master_read_logs(int fd, uint8_t *logs, size_t len);
void i2c_master_print_logs(uint8_t *logs, size_t len);

#endif // I2C_MASTER_H
