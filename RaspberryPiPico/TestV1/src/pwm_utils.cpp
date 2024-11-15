#include "pwm_utils.h"

void setup_pwm(uint gpio_pin, float freq) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

    const uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    // Calculate the PWM frequency and set the PWM divider
    float clock_freq = 125000000.0f;  // Default Pico clock frequency in Hz
    uint32_t divider = static_cast<uint32_t>(clock_freq / (freq * 65536));  // Compute divider for given frequency
    pwm_set_clkdiv(slice_num, divider);

    // Set the PWM wrap value (maximum count value)
    pwm_set_wrap(slice_num, 65535);  // 16-bit counter (0 - 65535)
}

void enable_pwm(uint gpio_pin, float duty_cycle) {
    const uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    // Set the PWM duty cycle level
    pwm_set_gpio_level(gpio_pin, static_cast<uint16_t>(duty_cycle * 65535));
    pwm_set_enabled(slice_num, true);
}

void disable_pwm(uint gpio_pin) {
    const uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    pwm_set_enabled(slice_num, false);
}


void setup_servo(uint servo_pin, uint start_angle)
{
    setup_pwm(servo_pin, 50);
    set_servo_angle(servo_pin, start_angle);
}

void set_servo_angle(uint servo_pin, uint angle)
{
    // Ensure angle is within valid range
    if (angle > 180) angle = 180;
    else if (angle < 0) angle = 0;
    
    enable_pwm(servo_pin, (angle/180.0f + 1)/20.0f);
}


void setup_L9110S_motor_driver(uint motorA_pin, uint motorB_pin)
{
    setup_pwm(motorA_pin, 400);
    setup_pwm(motorB_pin, 400);

    enable_pwm(motorA_pin, 0.0f);
    enable_pwm(motorB_pin, 0.0f);
}

void set_L9110S_motor_speed(uint motorA_pin, uint motorB_pin, float speed)
{
    if (speed == 0.0f) {
        enable_pwm(motorA_pin, 0.0f);
        enable_pwm(motorB_pin, 0.0f);
    } else if (speed > 0) {
        enable_pwm(motorB_pin, 0.0f);
        enable_pwm(motorA_pin, speed);
    } else {
        enable_pwm(motorA_pin, 0.0f);
        enable_pwm(motorB_pin, -speed);
    }
}