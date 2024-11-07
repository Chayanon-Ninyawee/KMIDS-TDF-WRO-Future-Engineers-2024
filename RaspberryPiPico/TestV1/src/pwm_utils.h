#ifndef PWM_UTILS_H
#define PWM_UTILS_H

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

// Function to set up the PWM
void setup_pwm(uint gpio_pin, float freq);

// Function to enable PWM with a specified duty cycle
void enable_pwm(uint gpio_pin, float duty_cycle);

// Function to disable PWM
void disable_pwm(uint gpio_pin);


// Function to set up Servo
void setup_servo(uint servo_pin, uint start_angle);

// Function to set Servo angle
void set_servo_angle(uint servo_pin, uint angle);


// Function to set up L9110S Motor Driver
void setup_L9110S_motor_driver(uint motorA_pin, uint motorB_pin);

// Function to set up L9110S Motor Driver speed
void set_L9110S_motor_speed(uint motorA_pin, uint motorB_pin, float speed);


#endif // PWM_UTILS_H
