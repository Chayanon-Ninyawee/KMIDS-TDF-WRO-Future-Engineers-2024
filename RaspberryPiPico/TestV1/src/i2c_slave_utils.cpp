#include "pico/i2c_slave.h"

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE:
        switch (i2c_read_byte_raw(i2c)) {
        case 0: // Set bno055 offset and skip calibration

            break;
        case 1: // Get bno055 offset

            break;
        case 2: // Get bno055 reading

            break;
        default:
            break;
        }
    case I2C_SLAVE_REQUEST:
        break;
    case I2C_SLAVE_FINISH:
        break;
    default:
        break;
    }
}