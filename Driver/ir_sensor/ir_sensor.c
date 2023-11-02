#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <ctype.h>

//Pins
#define IR_SENSOR_PIN 26

//Thresholds for White and Black Surface
#define WHITE_THRESHOLD 400 
#define BLACK_THRESHOLD 3000 

//Define for sensor value
uint16_t sensorValue;
//volatile uint16_t sensorValue;

//Barcode made out of bars (Black) and spaces (white)
//Starting Value
//Barcode scanning will start when black is sensed 
//for a period of time that is considered a barcode bar
absolute_time_t startBarcodeTime;
absolute_time_t endBarcodeTime;

int checkSurfaceColour(uint16_t data){
    if(data <= WHITE_THRESHOLD){
        //Sensor is on white surface, on track 
        printf("White surface sensed.\n");
        return 0;
    }
    else(sensorValue >= BLACK_THRESHOLD){
        //Sensor has touched black line
        printf("Black surface sensed.\n");
        return 1;
    }
    return 0;
}

int main(void) {
    stdio_init_all();
    adc_init();

    adc_gpio_init(IR_SENSOR_PIN);
    adc_select_input(0);

    gpio_set_dir_all_bits(0);
    for (int i = 2; i < 30; ++i) {
        gpio_set_function(i, GPIO_FUNC_SIO);
        if (i >= 26) {
            gpio_disable_pulls(i);
            gpio_set_input_enabled(i, false);
        }
    }

    colourBlack = false;

    while (1)
    {

        sensorValue = adc_read();
        printf("Original IR Sensor Reading: %d\n", sensorValue);
        int colour = checkSurfaceColour(sensorValue);

        //If black colour is sensed and it is the first time sensed
        if(colour == 1 && firstChange == true){
            startBarcodeTime = get_absolute_time();
            colourBlack = true;
            firstChange = false;
        }
        else if(colour == 1 && firstChange == false){

        }
        else if(colour == 0 && firstChange == false){
            star
        }

        //Measure pulse width for bar code

        sleep_ms(25);
    }

    return 0;

}


