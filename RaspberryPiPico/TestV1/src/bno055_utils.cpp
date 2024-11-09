#include "bno055_utils.h"

#include "debug_print.h"


int8_t bno055_initialize(bno055_t *bno, i2c_inst_t *i2c) {
    int8_t res = bno055_pico_init(bno, i2c_default, BNO055_I2C_ADDR1);
    if (res) {
        res = bno055_pico_init(bno, i2c_default, BNO055_I2C_ADDR2);
    }
    if (res) {
        DEBUG_PRINT("BNO055 initialization failed!\n");
        return res;
    }
    sleep_ms(100);

    res = bno055_set_power_mode(BNO055_POWER_MODE);
    if (res) {
        DEBUG_PRINT("BNO055 power mode set failed!\n");
        return res;
    }
    sleep_ms(100);

    res = bno055_set_operation_mode(BNO055_OPERATION_MODE);
    if (res) {
        DEBUG_PRINT("BNO055 operation mode set failed!\n");
        return res;
    }

    return res;
}

int8_t bno055_calibrate(bno055_gyro_offset_t *gyroOffset, 
                        bno055_accel_offset_t *accelOffset, 
                        bno055_mag_offset_t *magOffset) {
    uint8_t sysCalibrationStatus, gyroCalibrationStatus, accelCalibrationStatus, magCalibrationStatus;

    while (true) {
        bno055_get_sys_calib_stat(&sysCalibrationStatus);
        bno055_get_gyro_calib_stat(&gyroCalibrationStatus);
        bno055_get_accel_calib_stat(&accelCalibrationStatus);
        bno055_get_mag_calib_stat(&magCalibrationStatus);

        DEBUG_PRINT("Calibration Status - sys: %u, gyro: %u, accel: %u, mag: %u\n",
            sysCalibrationStatus, gyroCalibrationStatus, accelCalibrationStatus, magCalibrationStatus);

        if (sysCalibrationStatus == 3 && gyroCalibrationStatus == 3 && 
            accelCalibrationStatus == 3 && magCalibrationStatus == 3) {
            break;
        }

        sleep_ms(200);
    }


    int8_t res = bno055_set_operation_mode(BNO055_OPERATION_MODE_CONFIG);
    if (res) {
        DEBUG_PRINT("BNO055 operation mode failed to set to CONFIG!\n");
        return res;
    }
    sleep_ms(100);

    bno055_read_gyro_offset(gyroOffset);
    bno055_read_accel_offset(accelOffset);
    bno055_read_mag_offset(magOffset);

    DEBUG_PRINT("Fully calibrated!\n");
    DEBUG_PRINT("--------------------------------\n");
    DEBUG_PRINT("Calibration Results:\n");
    DEBUG_PRINT("gyro_offset - x: %d, y: %d, z: %d\n", gyroOffset->x, gyroOffset->y, gyroOffset->z);
    DEBUG_PRINT("accel_offset - x: %d, y: %d, z: %d, radius: %d\n", accelOffset->x, accelOffset->y, accelOffset->z, accelOffset->r);
    DEBUG_PRINT("mag_offset - x: %d, y: %d, z: %d, radius: %d\n", magOffset->x, magOffset->y, magOffset->z, magOffset->r);
    DEBUG_PRINT("--------------------------------\n");

    res = bno055_set_operation_mode(BNO055_OPERATION_MODE);
    if (res) {
        DEBUG_PRINT("BNO055 operation mode failed to set to CONFIG!\n");
        return res;
    }

    return 0;
}


int8_t bno055_load_offset(bno055_gyro_offset_t *gyroOffset, 
                        bno055_accel_offset_t *accelOffset, 
                        bno055_mag_offset_t *magOffset) {
    int8_t res = bno055_set_operation_mode(BNO055_OPERATION_MODE_CONFIG);
    if (res) {
        DEBUG_PRINT("BNO055 operation mode failed to set to CONFIG!\n");
        return res;
    }
    sleep_ms(100);

    bno055_write_gyro_offset(gyroOffset);
    bno055_write_accel_offset(accelOffset);
    bno055_write_mag_offset(magOffset);

    res = bno055_set_operation_mode(BNO055_OPERATION_MODE);

    return 0;
}