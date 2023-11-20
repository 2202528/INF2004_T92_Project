#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

// In controls the direction, en controls the speed of the motor
#define IN1 6
#define IN2 7
#define EN_A 8
#define IN3 4
#define IN4 3
#define EN_B 2


// Constants for your PID control
const float target_value = 500.0;  // The target sensor value you want to achieve
const float kp = 8.0;  // The proportional gain (adjust as needed)
const float ki = 0.325; // The integral gain (adjust as needed) //0.02
const float kd = 0.0003; // The derivative gain (adjust as needed) (when value goes up LEFT goes faster)

// Variables for PID control
float pre_error = 0.0; // Previous error
float integral = 0.0;  // Integral sum

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

    // Find out which PWM slice is connected to EN_A and EN_B 
    uint slice_num_A = pwm_gpio_to_slice_num(EN_A);
    uint slice_num_B = pwm_gpio_to_slice_num(EN_B);

    // PWM is how fast the signal oscilate between high and low
    // pwm set wrap is the period of the pwm signal which here is 12500 which is equal to 10ms
    pwm_set_wrap(slice_num_A, 12450); // Adjust this to set the desired frequency (Right Wheel) POV: bottom!
    pwm_set_wrap(slice_num_B, 12200); // Adjust this to set the desired frequency (Left wheel) //12325

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

// Function to calculate the error
float calculate_error(float current_value) {
    return target_value - current_value;

}

int main() {
    stdio_init_all();

    init_motors();


    //speed for A = 10000 , b = 10300  for alightnment of wheels (slow)
    while (1) {

    // Read the current sensor value (you need to implement this)
    // float current_value = read_sensor_value();  // Replace with your sensor reading function
    float current_value = 50.0; //this can be send to gauge distance, further current value from the target value, the wheels will move faster (TEST WITH 1000)

    // Calculate the error (the difference between the target and current values)
    float error = calculate_error(current_value);

    // Calculate the integral term
    integral += error;

    // Calculate the derivative term
    float derivative = error - pre_error;

    // Calculate the PID control signal
    float control_signal = (kp * error) + (ki * integral) + (kd * derivative);

    // Limit the control signal to stay within a reasonable range
    if (control_signal > 10000.0) {
        control_signal = 10000.0;
    } else if (control_signal < -10000.0) {
        control_signal = -10000.0;
    }

    // Set the motor speed using the control signal
    set_motor_speed(pwm_gpio_to_slice_num(EN_A), (int)control_signal);
    set_motor_speed(pwm_gpio_to_slice_num(EN_B), (int)control_signal);
    move_backward();

    // Update previous error for the next iteration
    pre_error = error;
        
        }

    return 0;
}
