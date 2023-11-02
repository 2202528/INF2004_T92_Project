#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//function to initialize I2C
void initialiseI2C() {
    i2c_init(i2c0,400000); //400kHz
    gpio_set_function(21, GPIO_FUNC_I2C);
    gpio_set_function(20, GPIO_FUNC_I2C);
    gpio_pull_up(21);
    gpio_pull_up(20);
}

void configureMagnetometer() {
    uint8_t magnetoAddress=0x1E;
    //0x00 means configure device operation, 
    //including output data rate 
    //and measurement configuration

    //0x02 means set operating mode such as continous measurement, single measurement or idle mode
    uint8_t configureMagnetometer[]={0x02,0x00};

    //Identification
    uint8_t configureCra[]={0x00,0x10};

    if (i2c_write_blocking(i2c0, magnetoAddress, configureMagnetometer, 2, false) != PICO_ERROR_GENERIC) {
        i2c_write_blocking(i2c0, magnetoAddress, configureCra, 2, false);
    }
    sleep_ms(10);
}

void readMagnetoMeterData() {
    uint8_t magnetoAddress=0x1E;
    uint8_t magnetoDataReg=0x03;

    if(i2c_write_blocking(i2c0, magnetoAddress, &magnetoDataReg, 1, true) !=PICO_ERROR_GENERIC) {
        uint8_t magnetoData[6]={0};
        if(i2c_read_blocking(i2c0, magnetoAddress, magnetoData, 6, false) !=PICO_ERROR_GENERIC) {
            int16_t x = (magnetoData[0] << 8) | magnetoData[1];
            int16_t y = (magnetoData[2] << 8) | magnetoData[3];
            int16_t z = (magnetoData[4] << 8) | magnetoData[5];

            if(x > 32767) {
                x -= 65536;
            } 
            else if(y > 32767){
                y -= 65536;
            }
            else if(z > 32767){
                z -= 65536;
            }

            printf("Magnetometer Readings: X: %d, Y: %d, Z: %d\n", x, y, z);
        }
        else{
            printf("Error with Magnetometer data!\n");
        }

    }
}

int main() {
    stdio_init_all();

    initialiseI2C();

    while(1) {
        configureMagnetometer();
        readMagnetoMeterData();
        sleep_ms(200);
    }
    return 0;
}

            