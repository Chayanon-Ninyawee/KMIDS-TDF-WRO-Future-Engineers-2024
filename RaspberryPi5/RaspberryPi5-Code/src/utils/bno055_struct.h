#ifndef BNO055_STRUCT_H
#define BNO055_STRUCT_H

/*!
 * @brief struct for Accel-output data of precision float
 */
struct bno055_accel_float_t {
    float x; /**< accel x float data */
    float y; /**< accel y float data */
    float z; /**< accel z float data */
};

/*!
 * @brief struct for Euler-output data of precision float
 */
struct bno055_euler_float_t {
    float h; /**< Euler h float data */
    float r; /**< Euler r float data */
    float p; /**< Euler p float data */
};

#endif  // BNO055_STRUCT_H