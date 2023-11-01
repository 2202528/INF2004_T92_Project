#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define SENSOR_PIN 2  // Pin connected to the wheel sensor 

//wheel and sensor characteristics
#define PULSES_PER_REV 360       //360 pulses per wheel rotation
#define WHEEL_SIZE 21.0  //wheel circ.

volatile uint32_t pulse_count = 0;
volatile uint32_t last_pulse_time = 0;

void sensor_interrupt() {
    pulse_count++;
    uint32_t current_time = time_us_32();
    uint32_t time_since_last_pulse = current_time - last_pulse_time;
    
    if (time_since_last_pulse > 0) {
        float rotation_speed = 1.0 / (time_since_last_pulse * 1e-6);  // Speed in Hz
        float linear_speed_cm_s = rotation_speed * (WHEEL_SIZE / PULSES_PER_REV);  // Speed in cm/s
        float traveled_distance_cm = (float)pulse_count * (WHEEL_SIZE / PULSES_PER_REV);  // Distance in cm
        printf("Dist: %.2f cm, Speed: %.2f cm/s\n", traveled_distance_cm, linear_speed_cm_s);
    }
    
    last_pulse_time = current_time;
}

int main() {
    stdio_init_all();
    gpio_init(SENSOR_PIN);
    gpio_set_dir(SENSOR_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &sensor_interrupt);
    
    while (1) {
        tight_loop_contents();
    }
    
    return 0;
}