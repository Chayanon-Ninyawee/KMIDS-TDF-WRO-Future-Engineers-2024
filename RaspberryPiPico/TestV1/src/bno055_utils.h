#ifndef BNO055_UTILS_H
#define BNO055_UTILS_H

#include <stdio.h>

#include "pico/stdlib.h"
#include "bno055.h"


const u8 BNO055_POWER_MODE = BNO055_POWER_MODE_NORMAL;
const u8 BNO055_OPERATION_MODE = BNO055_OPERATION_MODE_NDOF;


typedef struct bno055_t bno055_t;
typedef struct bno055_gyro_offset_t bno055_gyro_offset_t;
typedef struct bno055_accel_offset_t bno055_accel_offset_t;
typedef struct bno055_mag_offset_t bno055_mag_offset_t;
typedef struct bno055_accel_float_t bno055_accel_float_t;
typedef struct bno055_euler_float_t bno055_euler_float_t;


int8_t bno055_initialize(bno055_t *bno, i2c_inst_t *i2c);
int8_t bno055_calibrate(bno055_gyro_offset_t *gyroOffset, 
                        bno055_accel_offset_t *accelOffset, 
                        bno055_mag_offset_t *magOffset);
int8_t bno055_load_offset(bno055_gyro_offset_t *gyroOffset, 
                        bno055_accel_offset_t *accelOffset, 
                        bno055_mag_offset_t *magOffset);

#endif // BNO055_UTILS_H
