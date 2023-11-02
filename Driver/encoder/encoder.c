#include <stdio.h>  // Include the standard input/output library for printing
#include "pico/stdlib.h"  // Include the Raspberry Pi Pico SDK standard library
#include "hardware/gpio.h"  // Include the GPIO hardware library

#define SENSOR_PIN 2  // Pin connected to the wheel sensor

// Wheel and sensor characteristics
#define PULSES_PER_REV 360       // 360 pulses per wheel rotation
#define WHEEL_SIZE 21.0  // Wheel circumference centimeters

volatile uint32_t pulse_count = 0;  // Variable to keep track of pulse count
volatile uint32_t last_pulse_time = 0;  // Variable to keep track of the time of the last pulse

// Interrupt service routine (ISR) for the wheel sensor
void sensor_interrupt() {
    pulse_count++;  // Increment pulse count
    uint32_t current_time = time_us_32();  // Get the current time in microseconds
    uint32_t time_since_last_pulse = current_time - last_pulse_time;  // Calculate time since the last pulse

    if (time_since_last_pulse > 0) {
        // Calculate rotation speed in Hz
        float rotation_speed = 1.0 / (time_since_last_pulse * 1e-6);

        // Calculate linear speed in centimeters per second
        float linear_speed_cm_s = rotation_speed * (WHEEL_SIZE / PULSES_PER_REV);

        // Calculate the distance traveled in centimeters
        float traveled_distance_cm = (float)pulse_count * (WHEEL_SIZE / PULSES_PER_REV);

        // Print the calculated values
        printf("Dist: %.2f cm, Speed: %.2f cm/s\n", traveled_distance_cm, linear_speed_cm_s);
    }

    last_pulse_time = current_time;  // Update the time of the last pulse
}

int main() {
    stdio_init_all();  // Initialize standard I/O
    gpio_init(SENSOR_PIN);  // Initialize the GPIO pin connected to the sensor
    gpio_set_dir(SENSOR_PIN, GPIO_IN);  // Set the pin direction as input

    // Configure an interrupt on the SENSOR_PIN to trigger on a rising edge
    // and specify the sensor_interrupt function as the callback
    gpio_set_irq_enabled_with_callback(SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &sensor_interrupt);

    while (1) {
        tight_loop_contents();  // Keep the program running and handle sensor data through interrupts
    }

    return 0;  // Return 0 to indicate successful program execution
}
