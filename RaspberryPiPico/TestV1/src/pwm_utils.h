#ifndef PWM_UTILS_H
#define PWM_UTILS_H

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"


/**
 * @brief Configures a GPIO pin for PWM output with the specified frequency.
 * 
 * @param gpio_pin GPIO pin number to be configured for PWM.
 * @param freq Desired PWM frequency in Hz.
 * 
 * This function sets up the PWM configuration by assigning the GPIO pin to a PWM slice
 * and calculating the required clock divider to achieve the target frequency.
 */
void setup_pwm(uint gpio_pin, float freq);

/**
 * @brief Enables PWM output on a specified GPIO pin with a defined duty cycle.
 * 
 * @param gpio_pin GPIO pin number for PWM output.
 * @param duty_cycle Duty cycle as a fraction (0.0 - 1.0) where 1.0 is 100%.
 * 
 * Sets the PWM duty cycle to a specific level and enables PWM output on the specified pin.
 */
void enable_pwm(uint gpio_pin, float duty_cycle);

/**
 * @brief Disables PWM output on a specified GPIO pin.
 * 
 * @param gpio_pin GPIO pin number where PWM is to be disabled.
 * 
 * This function disables PWM output by turning off the PWM slice associated with the GPIO pin.
 */
void disable_pwm(uint gpio_pin);



/**
 * @brief Initializes a GPIO pin for servo control and sets the initial angle.
 * 
 * @param servo_pin GPIO pin number connected to the servo.
 * @param start_angle Initial angle of the servo in degrees (0 - 180).
 * 
 * Configures the PWM for a servo by setting a 50 Hz frequency and positions it to a specified angle.
 */
void setup_servo(uint servo_pin, uint start_angle);

/**
 * @brief Sets the angle of a servo motor connected to a specified GPIO pin.
 * 
 * @param servo_pin GPIO pin number connected to the servo.
 * @param angle Desired angle in degrees (0 - 180).
 * 
 * Adjusts the PWM duty cycle to position the servo to the specified angle.
 */
void set_servo_angle(uint servo_pin, uint angle);


/**
 * @brief Configures two GPIO pins for use with an L9110S motor driver.
 * 
 * @param motorA_pin GPIO pin connected to the motor driver's A input.
 * @param motorB_pin GPIO pin connected to the motor driver's B input.
 * 
 * Initializes the motor driver by setting up PWM on both GPIO pins and setting their duty cycle to 0.
 */
void setup_L9110S_motor_driver(uint motorA_pin, uint motorB_pin);

/**
 * @brief Controls the speed and direction of a motor connected to an L9110S driver.
 * 
 * @param motorA_pin GPIO pin connected to the motor driver's A input.
 * @param motorB_pin GPIO pin connected to the motor driver's B input.
 * @param speed Motor speed as a fraction (-1.0 to 1.0), where positive values represent forward
 *        rotation and negative values represent reverse rotation.
 * 
 * Adjusts the PWM duty cycle on the motor driver inputs to control motor speed and direction.
 * Setting speed to 0 stops the motor.
 */
void set_L9110S_motor_speed(uint motorA_pin, uint motorB_pin, float speed);


#endif // PWM_UTILS_H
