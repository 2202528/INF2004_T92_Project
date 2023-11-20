
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

#define GPIO_PIN 26
#define WHITE_THRESHOLD 400
#define BLACK_THRESHOLD 3000

//Number of switches between black and white before timer reset
#define INTERRUPT_LIMIT 30
#define CHAR_LEN 29

uint32_t edgeRise = 0x8u;
uint32_t edgeFall = 0x4u;

uint startTime = 0;
uint runningTime = 0;

//Falling Edge = Black Start, White End
//Rising Edge = White Start, Black End

int barcodeArray[CHAR_LEN]
= {0,0,0,0,0,0,0,0,0,
0,
0,0,0,0,0,0,0,0,0,
0,
0,0,0,0,0,0,0,0,0};

uint32_t barcodeTimings[CHAR_LEN][2];
//barcodeTimings[i][0] = black or white
//Black = 1, White = 0
//barcodeTimings[i][1] = timing;
uint32_t barcodeIndex = 0;
//int interruptNum = 0;

//Stopwatch will run at the first bar, will end at end of barcode
volatile bool stopwatchRunning = false;
volatile bool readingBarcode = false;
volatile bool completeBarcode = false;

//Thick  Bar = 3
//Thin  Bar = 1
//A = 3 1 1 1 1 3 1 1 3
//* = 1 3 1 1 3 1 3 1 1

//Thin White Bar = 0
//Thin Black Bar = 1
//Thick White Bar = 2
//Thin White Bar = 3

// const int barLetterA[INTERRUPT_LIMIT - 1] 
// = {1,2,1,0,3,0,3,0,1,
// 0,
// 3,0,1,0,1,2,1,0,3,
// 0,
// 1,2,1,0,3,0,3,0,1};

//THIN-WHITE: 0 (Binary:0)
//THIN-BLACK: 1 (Binary:1)
//THICK-WHITE: 2 (Binary: 000)
//THICK-BLACK: 3 (Binary: 111)

typedef struct BarLetter BarLetter;

struct BarLetter{
        int letter[CHAR_LEN];
};

//0-25, size = 26
enum BarLetters{
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z
};

BarLetter barLetters[26] = {
    // = 1 000 1 0 111 0 111 0 1
    // = 1 2   1 0 3   0 3   0 1  

    //111 0 1 0 1 000 1 0 111  
    //3   0 1 0 1 2   1 0 3

    //A
        [A]={1,2,1,0,3,0,3,0,1,
                0,
                3,0,1,0,1,2,1,0,3,
                0,
                1,2,1,0,3,0,3,0,1
        },

    //1 0 111 0 1 000 1 0 111
    //1 0 3   0 1 2   1 0 3 

    //B
    [B]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,1,2,1,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 111 0 1 000 1 0 1  
    //3   0 3   0 1 2   1 0 1
    //C
    [C]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,3,0,1,2,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 111 000 1 0 111
    //1 0 1 0 3   2   1 0 3
    //D
    [D]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,3,2,1,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 1 0 111 000 1 0 1
    //3   0 1 0 3   2   1 0 1
    //E
        [E]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,1,0,3,2,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 111 0 111 000 1 0 1 
    //1 0 3   0 3   2   1 0 1
    //F = 
    [F]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,3,2,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 1 000 111 0 111
    //1 0 1 0 1 2   3   0 3
    //G = 
    [G]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,1,2,3,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 1 0 1 000 111 0 1
    //3   0 1 0 1 2   3   0 1
    //H = 
    [H]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,1,0,1,2,3,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 111 0 1 000 111 0 1
    //1 0 3   0 1 2   3   0 1 
    //I = 
    [I]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,1,2,3,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 111 000 111 0 1
    //1 0 1 0 3   2   3   0 1
    //J = 
    [J]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,3,2,3,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 1 0 1 0 1 000 111
    //3   0 1 0 1 0 1 2   3 
    //K = 
    [K]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,1,0,1,0,1,2,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 111 0 1 0 1 000 111
    //1 0 3   0 1 0 1 2   3
    //L = 
    [L]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,1,0,1,2,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 111 0 1 0 1 000 1
    //3   0 3   0 1 0 1 2   1 
    //M = 
    [M]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,3,0,1,0,1,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 111 0 1 000 111 
    //1 0 1 0 3   0 1 2   3
    //N = 
    [N]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,3,0,1,2,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 1 0 111 0 1 000 1
    //3   0 1 0 3   0 1 2   1
    //O = 
    [O]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,1,0,3,0,1,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 111 0 111 0 1 000 1 
    //1 0 3   0 3   0 1 2   1
    //P = 
    [P]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,3,0,1,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 1 0 111 000 111
    //1 0 1 0 1 0 3   2   3
    //Q = 
    [Q]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,1,0,3,2,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 0 1 0 1 0 111 000 1
    //3   0 1 0 1 0 3   2   1 
    //R = 
    [R]={1,2,1,0,3,0,3,0,1,
            0,
            3,0,1,0,1,0,3,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 111 0 1 0 111 000 1
    //1 0 3   0 1 0 3   2   1
    //S = 
    [S]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,3,0,1,0,3,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 0 1 0 111 0 111 000 1 
    //1 0 1 0 3   0 3   2   1
    //T = 
    [T]={1,2,1,0,3,0,3,0,1,
            0,
            1,0,1,0,3,0,3,2,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 000 1 0 1 0 1 0 111
    //3   2   1 0 1 0 1 0 3 
    //U = 
    [U]={1,2,1,0,3,0,3,0,1,
            0,
            3,2,1,0,1,0,1,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 000 111 0 1 0 1 0 111
    //1 2   3   0 1 0 1 0 3   
    //V = 
    [V]={1,2,1,0,3,0,3,0,1,
            0,
            1,2,3,0,1,0,1,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 000 111 0 1 0 1 0 1
    //3   2   3   0 1 0 1 0 1 
    //W = 
    [W]={1,2,1,0,3,0,3,0,1,
            0,
            3,2,3,0,1,0,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 000 1 0 111 0 1 0 111
    //1 2   1 0 3   0 1 0 3
    //X = 
    [X]={1,2,1,0,3,0,3,0,1,
            0,
            1,2,1,0,3,0,1,0,3,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //111 000 1 0 111 0 1 0 1
    //3   2   1 0 3   0 1 0 1 
    //Y = 
    [Y]={1,2,1,0,3,0,3,0,1,
            0,
            3,2,1,0,3,0,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    },

    //1 000 111 0 111 0 1 0 1 
    //1 2   3   0 3   0 1 0 1
    //Z = 
    [Z]={1,2,1,0,3,0,3,0,1,
            0,
            1,2,3,0,3,0,1,0,1,
            0,
            1,2,1,0,3,0,3,0,1
    }
};

char barLetterToChar(enum BarLetters letter){
        switch (letter)
        {
        case A: return 'A';
        case B: return 'B';
        case C: return 'C';
        case D: return 'D';
        case E: return 'E';
        case F: return 'F';
        case G: return 'G';
        case H: return 'H';
        case I: return 'I';
        case J: return 'J';
        case K: return 'K';
        case L: return 'L';
        case M: return 'M';
        case N: return 'N';
        case O: return 'O';
        case P: return 'P';
        case Q: return 'Q';
        case R: return 'R';
        case S: return 'S';
        case T: return 'T';
        case U: return 'U';
        case V: return 'V';
        case W: return 'W';
        case X: return 'X';
        case Y: return 'Y';
        case Z: return 'Z';
        }
};

enum BarLetters barcodeMatch(int array[CHAR_LEN]) {
    // Loop through BarLetter array
    for (int i = 0; i < sizeof(barLetters) / sizeof(barLetters[0]); i++) {
        // Loop through the values array inside each BarLetter
        int matchCount = 0;
        for (int j = 0; j < sizeof(barLetters[i].letter) / sizeof(barLetters[i].letter[0]); j++) {
            if (barLetters[i].letter[j] == array[j]) {
                matchCount++;
            }
        }

        // Check if all elements match
        if (matchCount == sizeof(barLetters[i].letter) / sizeof(barLetters[i].letter[0])) {
            return (enum BarLetters)i;
        }
    }

    // Return a default value if no match is found
    return (enum BarLetters)-1;
};


char readBarcode(int array[CHAR_LEN]){
        enum BarLetters letter = barcodeMatch(array);
        if(letter == (enum BarLetters)-1){
                return '1';
        }
        else{
                char barChar = barLetterToChar(letter);
                return barChar;
        }
};

void copyArray(uint32_t original[CHAR_LEN][2], uint32_t copy[CHAR_LEN][2]){
        int len = CHAR_LEN;
        int col = 2;
        for(int i = 0; i < len; i++){
                for(int j = 0; j < col; j++){
                        copy[i][j] = original[i][j];
                }
        }
}

void sortArray(uint32_t arr[CHAR_LEN][2]) {
    for (int i = 0; i < CHAR_LEN - 1; i++) {
        for (int j = 0; j < CHAR_LEN - i - 1; j++) {
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

uint32_t findThreshold(uint32_t arr[CHAR_LEN][2] ){
    int len = CHAR_LEN;
    sortArray(arr); // Sort the array in ascending order

    uint32_t timings[len];

    for(int i = 0; i < CHAR_LEN; i++){
        timings[i] = arr[i][0];
    }

    if (len % 2 == 0) {
        return (timings[len / 2 - 1] + timings[len / 2]) / 2;
    } else {
        return timings[len / 2];
    }
}

//To Process Array, first must check if considered long and short timings
//Then check if considered thin or thick bars and what colour
//Then can assign number 0-3 to the specific bar/array timing
void processArray(uint32_t array[CHAR_LEN][2], int barcodeArr[CHAR_LEN]){
        //int barArray[CHAR_LEN];
        uint32_t copy[CHAR_LEN][2];
        copyArray(array, copy);
        uint32_t median = findThreshold(copy);

        for(int i = 0; i < CHAR_LEN; i++){
                //If White
                if(array[i][0] == 0){
                        //Thin White
                        if(array[i][1] < median){
                                barcodeArr[i] = 0;
                        }
                        //Thick White
                        else{
                                barcodeArr[i] = 2;
                        }
                }
                //If Black
                else if(array[i][0] == 1){
                        //Thin Black
                        if(array[i][1] < median){
                                barcodeArr[i] = 1;
                        }
                        //Thick Black
                        else{
                                barcodeArr[i] = 3;
                        }
                }
        }
}


//0-29

//*A* = 1 2 1 0 3 0 3 0 1 (*)
//0 
//3 0 1 0 1 2 1 0 3 
//0 
//1 2 1 0 3 0 3 0 1 
//0

//A = 
//* = 1 2 1 0 3 
//* = 1 000 1 0 111 0 111 0 1 0

//*A* 121030301 0 301012103 0 121030301 (missing 0 as stopwatch ends) (29 characters)

void gpio_callback(uint gpio, uint32_t events){
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    //uint32_t edgeRise = 0x8u;
    //uint32_t edgeFall = 0x4u;

    //The first interrupt is falling edge from HIGH to LOW
    //The last interrupt is rising edge from LOW back to HIGH

    printf("Interrupt Occured!\n");

    if(events == edgeFall && readingBarcode == false){
        //FROM HIGH TO LOW
        //From White to Black
        //Black Timing Starts
        //Barcode reading starts
        readingBarcode = true;
        printf("Barcode Reading Started\n");
        //interruptNum = 0;
        //interruptNum = interruptNum + 1;
        startTime = to_ms_since_boot(get_absolute_time());
    }
    else if(events == edgeFall && readingBarcode == true){
        //FROM HIGH TO LOW
        //From White to Black
        //White Timing Ends
        //Black Timing Starts
        if(stopwatchRunning){
            runningTime = to_ms_since_boot(get_absolute_time());
            barcodeTimings[barcodeIndex][0] = 0;
            barcodeTimings[barcodeIndex][1] = runningTime - startTime;
            startTime = runningTime;
            printf("Barcode Timing %d Saved!\n", barcodeIndex+1);
            printf("Barcode Bar is White!\n");
            barcodeIndex++;

        //     if(barcodeIndex => 30){
        //         stopwatchRunning = false;
        //         barcodeIndex = 0;
                
        //     }
        // }
        }

    }
    else if(events == edgeRise && readingBarcode == true){
        //check InterruptNum or timingindex
        //FROM LOW TO HIGH
        //From Black to White
        //Black Timing Ends
        //White Timing Starts
        if(stopwatchRunning){
            runningTime = to_ms_since_boot(get_absolute_time());
            barcodeTimings[barcodeIndex][0] = 1;
            barcodeTimings[barcodeIndex][1] = runningTime - startTime;
            startTime = runningTime;
                printf("Barcode Timing %d Saved!\n", barcodeIndex+1);
            printf("Barcode Bar is Black!\n");
            barcodeIndex++;

            if(barcodeIndex == CHAR_LEN){
                stopwatchRunning = false;
                readingBarcode = false;
                //completeBarcode = true;

                processArray(barcodeTimings, barcodeArray);
                char data = readBarcode(barcodeArray);
                if(strcmp(data, "1") == 0){
                        printf("Error reading barcode. Try again!\n");
                }
                else{
                        printf("The alphabet letter read was: %c\n", data);
                }
                
                barcodeIndex = 0;
                
                
            }
        }

        
    }
    
}

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//     printf("Timer %d fired!\n", (int) id);
//     timer_fired = true;
//     // Can return a value here in us to fire in the future
//     return 0;
// }

// bool repeating_timer_callback(struct repeating_timer *t) {
//     printf("Repeat at %lld\n", time_us_64());
//     return true;
// }

int main() {
    stdio_init_all();

    printf("Hello GPIO IRQ\n");
    printf("Hello Timer!\n");

    gpio_set_dir(GPIO_PIN, GPIO_IN);
    gpio_set_pulls(GPIO_PIN, true, false);

    //Rising Edge / Edge Rise = 0x8u
    //Falling Edge / Edge Fall = 0x4u

    gpio_set_irq_enabled_with_callback(GPIO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Wait forever
    while (1){
        if(!stopwatchRunning && readingBarcode == false){
            stopwatchRunning = true;
            barcodeIndex = 0;
            //startTime = time_us_32();
            startTime = to_ms_since_boot(get_absolute_time());
        }

        tight_loop_contents();
    };

    return 0;
};
