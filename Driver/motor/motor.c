#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

// Define motor pins
#define IN1 6
#define IN2 7
#define EN_A 8
#define IN3 4
#define IN4 3
#define EN_B 2

// Initialize motor pins
void init_motors() {
    gpio_init(IN1);
    gpio_init(IN2);
    gpio_init(IN3);
    gpio_init(IN4);
    
    gpio_set_dir(IN1, GPIO_OUT);
    gpio_set_dir(IN2, GPIO_OUT);
    gpio_set_dir(IN3, GPIO_OUT);
    gpio_set_dir(IN4, GPIO_OUT);

    // Initialize PWM for motor control
    gpio_set_function(EN_A, GPIO_FUNC_PWM);
    gpio_set_function(EN_B, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to EN_A (it's slice 0) and EN_B (it's slice 1)
    uint slice_num_A = pwm_gpio_to_slice_num(EN_A);
    uint slice_num_B = pwm_gpio_to_slice_num(EN_B);

    // Set PWM frequency (Hz)
    pwm_set_wrap(slice_num_A, 12500); // Adjust this to set the desired frequency
    pwm_set_wrap(slice_num_B, 12500); // Adjust this to set the desired frequency

    // Enable PWM slices
    pwm_set_enabled(slice_num_A, true);
    pwm_set_enabled(slice_num_B, true);
}

// Control motor speed using PWM
void set_motor_speed(uint slice, uint16_t duty_cycle) {
    pwm_set_chan_level(slice, PWM_CHAN_A, duty_cycle);
}

// Move forward
void move_forward() {
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);
    gpio_put(IN3, 1);
    gpio_put(IN4, 0);
}

// Move backward
void move_backward() {
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
}

// Turn right
void turn_right() {
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 1);
}

// Turn left
void turn_left() {
    gpio_put(IN1, 0);
    gpio_put(IN2, 1);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
}

// Stop
void stop() {
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), 0); // Stop motor A
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), 0); // Stop motor B
}

int main() {
    stdio_init_all();

    init_motors();


    //speed for A = 10000 , b = 10300  for alightnment of wheels (slow)
    while (1) {
    // move forward
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), 12000); // Adjust duty cycle for forward speed control
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), 12300); // Adjust duty cycle for forward speed control
    move_forward();
    sleep_ms(500);
    stop();
    sleep_ms(500);
    
    // // move backward
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), 10000); // Adjust duty cycle for backward speed control
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), 10300); // Adjust duty cycle for backward speed control
    move_backward();
    sleep_ms(500);
    stop();
    sleep_ms(500);
    
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), 12000); // Adjust duty cycle for backward speed control
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), 12300); // Adjust duty cycle for backward speed control
    turn_left();
    sleep_ms(500);
    stop();
    sleep_ms(500);    
    
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), 10000); // Adjust duty cycle for backward speed control
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), 10300); // Adjust duty cycle for backward speed control
    turn_right();
    sleep_ms(500);
    stop();
    sleep_ms(500);    
        
        }

    return 0;
}
