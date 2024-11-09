#include "i2c_slave_utils.h"

I2C_slave_state i2c_slave_state = {
  .recieved_command = command::DO_NOTHING,
  .is_command_set = false,
  .request_type = request_type_t::NOTHING,
  .recieved_bno055_calib_bytes = {.buffer = {0}, .address = 0},
  .recieved_info_bytes = {.buffer = {0}, .address = 0},
  .sending_bno055_calib_bytes = {.buffer = {0}, .address = 0},
  .sending_info_bytes = {.buffer = {0}, .address = 0},
  .restart_callback = nullptr,
  .bno055_calib_callback = nullptr,
  .load_bno055_calib_callback = nullptr,
  .get_info_callback = nullptr
};

void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE:
      if (not i2c_slave_state.is_command_set) {
        i2c_slave_state.recieved_command = (command)i2c_read_byte_raw(i2c);
        i2c_slave_state.is_command_set = true;
      } else {
        switch (i2c_slave_state.recieved_command) {
          case command::GET_BNO055_CALIB:
            if (i2c_slave_state.recieved_bno055_calib_bytes.address < sizeof(i2c_slave_state.recieved_bno055_calib_bytes.buffer)) {
              i2c_slave_state.recieved_bno055_calib_bytes.buffer[i2c_slave_state.recieved_bno055_calib_bytes.address++] = i2c_read_byte_raw(i2c);
            }
            break;
          case command::GET_INFO:
            if (i2c_slave_state.recieved_info_bytes.address < sizeof(i2c_slave_state.recieved_info_bytes.buffer)) {
              i2c_slave_state.recieved_info_bytes.buffer[i2c_slave_state.recieved_info_bytes.address++] = i2c_read_byte_raw(i2c);
            }
            break;
          case command::SEND_BNO055_CALIB:
            i2c_slave_state.request_type = request_type_t::BNO055_CALIB_DATA;
            break;
          case command::SEND_INFO:
            i2c_slave_state.request_type = request_type_t::INFO_DATA;
            break;
          default:
            break;
        }
      }

      break;
    
    case I2C_SLAVE_REQUEST:
      switch (i2c_slave_state.request_type) {
        case request_type_t::BNO055_CALIB_DATA:
          if (i2c_slave_state.sending_bno055_calib_bytes.address < sizeof(i2c_slave_state.sending_bno055_calib_bytes.buffer)) {
            i2c_write_byte_raw(i2c, i2c_slave_state.sending_bno055_calib_bytes.buffer[i2c_slave_state.sending_bno055_calib_bytes.address++]);
          }
          break;
        case request_type_t::INFO_DATA:
          if (i2c_slave_state.sending_info_bytes.address < sizeof(i2c_slave_state.sending_info_bytes.buffer)) {
            i2c_write_byte_raw(i2c, i2c_slave_state.sending_info_bytes.buffer[i2c_slave_state.sending_info_bytes.address++]);
          }
          break;
        default:
          break;
      }

      break;

    case I2C_SLAVE_FINISH:
      if (i2c_slave_state.is_command_set) {
        switch (i2c_slave_state.recieved_command) {
          case command::RESTART:
            if (i2c_slave_state.restart_callback) {
              i2c_slave_state.restart_callback();
            }
            break;
          case command::BNO055_CALIB:
            if (i2c_slave_state.bno055_calib_callback) {
              i2c_slave_state.bno055_calib_callback();
            }
            break;
          case command::GET_BNO055_CALIB:
            if (i2c_slave_state.load_bno055_calib_callback) {
              i2c_slave_state.load_bno055_calib_callback();
            }
            // memset(i2c_slave_state.recieved_bno055_calib_bytes.buffer, 0xFF, sizeof(i2c_slave_state.recieved_bno055_calib_bytes.buffer));
            i2c_slave_state.recieved_info_bytes.address = 0;
            break;
          case command::GET_INFO:
            if (i2c_slave_state.get_info_callback) {
              i2c_slave_state.get_info_callback();
            }
            // memset(i2c_slave_state.recieved_info_bytes.buffer, 0xFF, sizeof(i2c_slave_state.recieved_info_bytes.buffer));
            i2c_slave_state.recieved_info_bytes.address = 0;
            break;
          default:
            break;
        }
        i2c_slave_state.is_command_set = false;
      } else {
        switch (i2c_slave_state.request_type) {
          case request_type_t::BNO055_CALIB_DATA:
            i2c_slave_state.sending_bno055_calib_bytes.address = 0;
            break;
          case request_type_t::INFO_DATA:
            i2c_slave_state.sending_info_bytes.address = 0;
            break;
          default:
            break;
        }
        i2c_slave_state.request_type = request_type_t::NOTHING;
      }

      break;

    default:
      break;
  }
}


void set_bno055_offset_data(bno055_gyro_offset_t *gyroOffset, bno055_accel_offset_t *accelOffset, bno055_mag_offset_t *magOffset) {
  uint8_t *buffer_ptr = i2c_slave_state.sending_bno055_calib_bytes.buffer;
  memcpy(buffer_ptr, gyroOffset, sizeof(bno055_gyro_offset_t));
  memcpy(buffer_ptr + sizeof(bno055_gyro_offset_t), accelOffset, sizeof(bno055_accel_offset_t));
  memcpy(buffer_ptr + sizeof(bno055_gyro_offset_t) + sizeof(bno055_accel_offset_t), magOffset, sizeof(bno055_mag_offset_t));
}

void set_info_data(bno055_accel_float_t *accelData, bno055_euler_float_t *eulerAngles, char *logs) {
  bno055_euler_float_t angleData = *eulerAngles;
  bno055_convert_float_euler_hpr_deg(&angleData);

  memcpy(i2c_slave_state.sending_info_bytes.buffer, accelData, sizeof(bno055_accel_float_t));
  memcpy(i2c_slave_state.sending_info_bytes.buffer + sizeof(bno055_accel_float_t), &angleData, sizeof(bno055_euler_float_t));

  const size_t log_offset = sizeof(bno055_accel_float_t) + sizeof(bno055_euler_float_t);
  const size_t max_log_size = sizeof(I2C_slave_state::sending_info_bytes.buffer) - log_offset - 5; // Reserve 5 byte for ending marker
  
  // Determine the length of the log data and copy it
  size_t log_size = strnlen(logs, max_log_size);
  memcpy(i2c_slave_state.sending_info_bytes.buffer + log_offset, logs, log_size);

  // Set the next 5 bytes as the unique ending marker (0xFF)
  memset(i2c_slave_state.sending_info_bytes.buffer + log_offset + log_size, 0xFF, 5);
}


bool is_bno055_offset_data_ready() {
  return not std::all_of(i2c_slave_state.recieved_bno055_calib_bytes.buffer, i2c_slave_state.recieved_bno055_calib_bytes.buffer + sizeof(i2c_slave_state.recieved_bno055_calib_bytes.buffer), [](uint8_t byte) { return byte == 0xFF; });
}

void get_bno055_offset_data(bno055_gyro_offset_t *outputGyroOffset, bno055_accel_offset_t *outputAccelOffset, bno055_mag_offset_t *outputMagOffset) {
  if (not is_bno055_offset_data_ready()) return;

  memcpy(outputGyroOffset, i2c_slave_state.recieved_bno055_calib_bytes.buffer, sizeof(bno055_gyro_offset_t));
  memcpy(outputAccelOffset, i2c_slave_state.recieved_bno055_calib_bytes.buffer + sizeof(bno055_gyro_offset_t), sizeof(bno055_accel_offset_t));
  memcpy(outputMagOffset, i2c_slave_state.recieved_bno055_calib_bytes.buffer + sizeof(bno055_gyro_offset_t) + sizeof(bno055_accel_offset_t), sizeof(bno055_mag_offset_t));
}


bool is_info_data_ready() {
  return not std::all_of(i2c_slave_state.recieved_info_bytes.buffer, i2c_slave_state.recieved_info_bytes.buffer + sizeof(i2c_slave_state.recieved_info_bytes.buffer), [](uint8_t byte) { return byte == 0xFF; });
}

void get_info_data(float *outputMotorPercent, float *outputSteeringPercentage) {
  if (not is_info_data_ready()) return;

  memcpy(outputMotorPercent, i2c_slave_state.recieved_info_bytes.buffer, sizeof(float));
  memcpy(outputSteeringPercentage, i2c_slave_state.recieved_info_bytes.buffer + sizeof(float), sizeof(float));
}