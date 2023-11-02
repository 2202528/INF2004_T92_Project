#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

int timeout = 30000; // Increase the timeout to handle longer distances
#define TRIGGER_PIN 4 //trigger pin number
#define ECHO_PIN 5 //echopin number

uint64_t getPulse(uint trigPin, uint echoPin)
{
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

    uint64_t width = 0;

    while (gpio_get(echoPin) == 0) tight_loop_contents();
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(echoPin) == 1)
    {
        width++; //measure the duration of the echopulse
        sleep_us(1);
        if (width > timeout)
            return 0;
    }
    absolute_time_t endTime = get_absolute_time();

    return absolute_time_diff_us(startTime, endTime);
}

float getDistance(uint triggerPin, uint echoPin)
{
    uint64_t pulseLength = getPulse(triggerPin, echoPin); //returns legnth of time btw trig and echo
    if (pulseLength == 0)
        return -1.0; // Error condition

    // Calculate the distance in centimeters
    float dist = (float)pulseLength / 29 /2; 

    return dist;
}

int main()
{

    stdio_init_all();

    gpio_init(TRIGGER_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    while (1)
    {
        //Distance is in CM
        float dist = getDistance(TRIGGER_PIN, ECHO_PIN);
        if (dist >= 0)
        {
            printf("Distance: %.2f cm\n", dist);
        }
        else
        {
            printf("No object detected\n");
        }
        sleep_ms(1000); // Update the distance every second
    }

    return 0;
}



