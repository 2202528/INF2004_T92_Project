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

//Define Barcode Size
#define BARCODE_SIZE 47

//Define for sensor value
uint16_t sensorValue;

char* barcodeStr[BARCODE_SIZE + 1];

//Boolean to see if bar sensed
bool isBlackBar = false;
////Boolean if barcode is currently sensing;
bool sensingBarcode = false;
//Barcode Array Count
int barcodeCount = 0;

absolute_time_t startBarcodeTime;
absolute_time_t endBarcodeTime;

uint32_t startTime;
uint32_t endTime;
uint32_t pulseWidth;
//Timing Array
//[0] = pulsewidth
//[1] = black/white

uint32_t threshold;

const int barLetterA[BARCODE_SIZE] = {1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1};

void sortArray(uint32_t arr[][2], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j][0] > arr[j + 1][0]) {
                int temp1 = arr[j][0];
                int temp2 = arr[j][1];

                arr[j][0] = arr[j + 1][0];
                arr[j][1] = arr[j + 1][1];

                arr[j + 1][0] = temp1;
                arr[j + 1][1] = temp2;
            }
        }
    }
}


uint32_t findThreshold(uint32_t arr[][2], int size) {
    sortArray(arr, size); // Sort the array in ascending order

    uint32_t timings[size];

    for(int i = 0; i < size; i++){
        timings[i] = arr[i][0];
    }

    if (size % 2 == 0) {
        return (timings[size / 2 - 1] + timings[size / 2]) / 2;
    } else {
        return timings[size / 2];
    }
}

int* processBarcode(uint32_t array[][2], int size){
    uint32_t median = findThreshold(array, size);
    int barcode[BARCODE_SIZE];
    int i = 0;
    while (true)
    {
        if(i <= 47){
            //If White
            if(array[i][1] == 0){
                if(array[i][0] < median){
                    //Thin
                    barcode[i] = 0;
                    i++;
                    continue;
                }
                else{
                    //Thick
                    for (int j = 0; j < 3; j++)
                    {
                        barcode[i] = 0;
                        i++;
                    }
                    continue;
                }
            }
            //If Black
            else if(array[i][1] == 1){
                if(array[i][0] < median){
                    //Thin
                    barcode[i] = 1;
                    i++;
                    continue;
                }
                else{
                    //Thick
                    for (int j = 0; j < 3; j++)
                    {
                        barcode[i] = 1;
                        i++;
                    }
                    continue;
                }
            }
        }
        else{
            break;
        }
    }
    
    return barcode;
}

int checkIfA(int barCode[], int barLetter[BARCODE_SIZE]){
    for(int i = 0; i < BARCODE_SIZE; i++ ){
        if(barCode[i] != barLetter[i]){
            return 1;
        }
    }

    return 0;
}


int checkSurfaceColour(uint16_t data){
    if(data <= WHITE_THRESHOLD){
        //Surface is white 
        printf("0\n");
        return 0;
    }
    else if(data >= BLACK_THRESHOLD){
        //Surface is black
        printf("1\n");
        return 1;
    }
    return 0;
}

int checkWidth(uint32_t pulse, uint32_t threshold){
    //Thin = 0
    //Thick = 1
    if(pulse > threshold){
        return 0;
    }
    else{
        return 1;
    }

}

int main(void) {

    //Set up ADC Pin for IR Sensor
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

    //Initialise basic values
    startTime = 0;
    endTime = 0;
    pulseWidth = 0;
    isBlackBar = false;
    //Threshold for thickness
    //Threshold in us
    threshold = 50000;

    int barsCount = 0;
    uint32_t pulseArray[47][2];

    while (1)
    {
        //Read value sensed
        sensorValue = adc_read();
        //printf("Sensor Reading: %d\n", sensorValue);
        int colour = checkSurfaceColour(sensorValue);
        sleep_ms(1);
        if(barcodeCount <= 47){
            //Start scanning black
            if(colour == 1 && isBlackBar == false){
                endTime = time_us_32();
                //White space scanned completed
                pulseWidth = endTime - startTime;
                pulseArray[barsCount][0] = pulseWidth;
                pulseArray[barsCount][1] = 0;
                barsCount++;
                
                if(pulseWidth > 1000000){
                    sensingBarcode = true;
                    barcodeCount = 0;
                    
                    
                    //Restarting Barcode reading              
                    for(int i = 0; i < barsCount; i++){
                        pulseArray[i][0] = 0;
                        pulseArray[i][1] = 0;
                    }

                    barsCount = 0;
                    printf("Restarting Barcode Reading.\n");
                }

                isBlackBar = true;
                startTime = time_us_32();
            }
            //Start scanning white space
            else if(colour == 0 && isBlackBar == true){
                endTime = time_us_32();
                //Black bar scanned completed
                pulseWidth = endTime - startTime;
                pulseArray[barsCount][0] = pulseWidth;
                pulseArray[barsCount][1] = 1;
                barsCount++;
                if(pulseWidth > 1000000){
                    sensingBarcode = true;
                    barcodeCount = 0;
                    //Restarting Barcode reading
                    for(int i = 0; i < barsCount; i++){
                        pulseArray[i][0] = 0;
                        pulseArray[i][1] = 0;
                    }

                    barsCount = 0;
                }

                isBlackBar = false;
                startTime = time_us_32();
            }

        }
        else{
            
            uint32_t median = findThreshold(pulseArray, barsCount);
            int barcode[BARCODE_SIZE];
            int i = 0;
            while (true)
            {
                if(i <= 47){
                    //If White
                    if(pulseArray[i][1] == 0){
                        if(pulseArray[i][0] < median){
                            //Thin
                            barcode[i] = 0;
                            i++;
                            continue;
                        }
                        else{
                            //Thick
                            for (int j = 0; j < 3; j++)
                            {
                                barcode[i] = 0;
                                i++;
                            }
                            continue;
                        }
                    }
                    //If Black
                    else if(pulseArray[i][1] == 1){
                        if(pulseArray[i][0] < median){
                            //Thin
                            barcode[i] = 1;
                            i++;
                            continue;
                        }
                        else{
                            //Thick
                            for (int j = 0; j < 3; j++)
                            {
                                barcode[i] = 1;
                                i++;
                            }
                            continue;
                        }
                    }
                }
                else{
                    break;
                }
            }
            for(int loop = 0; loop < BARCODE_SIZE - 1; loop++){
                printf("%d", barcode[loop]);
            }

            printf("%d", barcode[BARCODE_SIZE -1]);
            
            //Check if barcode is A
            int ifA = checkIfA(barcode, barLetterA);
            if(ifA == 1){
                //Print error message
                printf("Error with reading barcode.\n");
            }
            else{
                //Barcode successfully read
                printf("Barcode read. Letter is A.\n");
            }
            barcodeCount = 0;
            //Restarting Barcode reading              
            for(int i = 0; i < barsCount; i++){
                pulseArray[i][0] = 0;
                pulseArray[i][1] = 0;
            }
            barsCount = 0;
        }
    }

    return 0;

}


