#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

//Pins for ADC Input
#define LEFT_ADC 26
#define RIGHT_ADC 27
//0-3 is 26-29, 0=26, 1=27, 2=28, 3=29
#define LEFT_ADC_INPUT 0
#define RIGHT_ADC_INPUT 1

//Pins for Digital Input
#define LEFT_DIGITAL 20
#define RIGHT_DIGITAL 21

// //Pins for VCC Input 
//VBUS and 3v3 out
//Use any GND

// #define LEFT_VCC 6 //for now can change later
// #define RIGHT_VCC 7 //for now can change later

// //Pins for GND Input
// #define LEFT_GND  4 //for now can change later
// #define RIGHT_GND 5//for now can change later

#define WHITE_THRESHOLD 400
#define BLACK_THRESHOLD 2000

//int to uint8_t if don't work
//void setUpIRPin(int pin, int vcc, int gnd){
void setUpIRPin(int pin){
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_input_enabled(pin, false);
    gpio_disable_pulls(pin);

    //gpio_set_dir(vcc, GPIO_OUT);
    //gpio_set_dir(gnd, GPIO_OUT);
    //gpio_put(vcc, 1);
    //gpio_put(gnd, 0);
    
}

bool isSurfaceBlack(uint16_t adcResult){
    if(adcResult > BLACK_THRESHOLD){
        return true;
    }
    else{
        return false;
    }
}

void printData(bool isLeft, bool isBlack, uint16_t adcData, bool digitalData ){
    
    //If Data is for Left IR Sensor
    if(isLeft){
        //Black Colour is sensed
        if(isBlack){
            printf("The Left Sensor sensed Black.\n");  
        }
        //White Colour is sensed
        else{
            printf("The Left Sensor sensed White.\n");
        }

        printf("The Left ADC Data: %u\n", adcData);
        //0 is false, 1 is true
        //0 is low, 1 is high
        if(digitalData){
            printf("The Left Digital Data: HIGH\n");
        }
        else{
            printf("The Left Digital Data: LOW\n");
        }
    }

    //If Data is for Right IR Sensor
    else{
        //Black Colour is sensed
        if(isBlack){
            printf("The Right Sensor sensed Black.\n");  
        }
        //White Colour is sensed
        else{
            printf("The Right Sensor sensed White.\n");
        }

        printf("The Right ADC Data: %u\n", adcData);
        //0 is false, 1 is true
        //0 is low, 1 is high
        if(digitalData){
            printf("The Right Digital Data: HIGH\n");
        }
        else{
            printf("The Right Digital Data: LOW\n");
        }
    }
}

int main(void) {

    // Initialize stdio
    stdio_init_all();
    adc_init();
    setUpIRPin(LEFT_ADC);
    setUpIRPin(RIGHT_ADC);

    //setUpIRPin(LEFT_ADC, LEFT_VCC, LEFT_GND);
    //setUpIRPin(RIGHT_ADC, RIGHT_VCC, RIGHT_GND);

    while(1){
        //adc_init();
        //Get ADC Data
        adc_select_input(LEFT_ADC_INPUT);
        uint16_t leftADCdata = adc_read();
        adc_select_input(RIGHT_ADC_INPUT);
        uint16_t rightADCdata = adc_read();

        //Get Digital Data
        bool leftDigitalData = gpio_get(LEFT_DIGITAL);
        bool rightDigitalData = gpio_get(RIGHT_DIGITAL);

        bool leftSideBlack = isSurfaceBlack(leftADCdata);
        bool rightSideBlack = isSurfaceBlack(rightADCdata);

        printData(true, leftSideBlack, leftADCdata, leftDigitalData);
        printData(false, rightSideBlack, rightADCdata, rightDigitalData);
        sleep_ms(100);
    }
    
    return 0;
}