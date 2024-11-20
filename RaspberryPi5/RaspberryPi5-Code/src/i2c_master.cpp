#include "i2c_master.h"

#include <unistd.h>
#include <wiringPiI2C.h>

#include <cstring>
#include <chrono>
#include <cstdio>
#include <thread>


int i2c_master_init(uint8_t slave_address) {
  int fd = wiringPiI2CSetup(slave_address);
  if (fd == -1) {
    printf("Failed to initialize I2C communication.\n");
    return -1;
  }
  printf("I2C communication successfully initialized with slave address 0x%X.\n", slave_address);
  return fd;
}

void i2c_master_send_command(int fd, uint8_t command) {
  uint8_t cmd[2] = {i2c_slave_mem_addr::COMMAND_ADDR, command};
  if (write(fd, cmd, sizeof(cmd)) == -1) {
    perror("Failed to send command");
  }
  // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // TODO: Try removing this
}

void i2c_master_send_data(int fd, uint8_t reg, uint8_t *data, uint8_t len) {
  uint8_t data_buffer[1+len];
  data_buffer[0] = reg;
  memcpy(&data_buffer[1], data, len);
  if (write(fd, data_buffer, sizeof(data_buffer)) == -1) {
    perror("Failed to send command");
  }
  // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // TODO: Try removing this
}

void i2c_master_read_data(int fd, uint8_t reg, uint8_t *data, uint8_t len) {
  if (write(fd, &reg, 1) == -1) {
    perror("Failed to set register address");
    return;
  }
  if (read(fd, data, len) == -1) {
    perror("Failed to read data");
  }
}

void i2c_master_read_logs(int fd, uint8_t *logs) {
  i2c_master_read_logs(fd, logs, i2c_slave_mem_addr::LOGS_BUFFER_SIZE);
}

void i2c_master_read_logs(int fd, uint8_t *logs, size_t len) {
  uint8_t reg = i2c_slave_mem_addr::LOG_ADDR;
  if (write(fd, &reg, 1) == -1) {
    perror("Failed to set log address");
    return;
  }
  if (read(fd, logs, len) == -1) {
    perror("Failed to read logs");
  }
}

void i2c_master_print_logs(uint8_t *logs, size_t len) {
  if (logs == NULL) {
    return; // Handle null pointer gracefully
  }

  for (int i = 0; i < len; i++) {
    if (logs[i] == 0xFF) {
      break;
    }

    printf("%c", logs[i]);
  }
}