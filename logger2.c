#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

// The GPIO pins we want to log
int PINS[] = {2, 3, 4};
int NB_PINS = (int) ((sizeof(PINS)) / (sizeof(PINS[0])));

void init_gpio() {
    /*
     * Setup all desired GPIO pins for use as input
     * and pull them to UP
     */
    
    // Setup GPIO usage
    wiringPiSetupGpio();

    // Set all desired GPIO pins to input mode and UP
    for (int i = 0; i <= NB_PINS; i++) {
        pinMode(PINS[i], INPUT);
        pullUpDnControl(PINS[i], PUD_UP);
    }
}

int* query_all() {
    /*init_gpio
     *Query all desired GPIO pins and return their status.
     */
    
    int* current_status = (int*) malloc(NB_PINS * sizeof(int));
    
    for (int i = 0; i < NB_PINS; i++) {
        current_status[i] = digitalRead(PINS[i]);
    }
    
    return current_status;
}

void print_array(int* x, int n) {
    /*
     * Helper function to print an int array of length n.
     */

    for (int i = 0; i < n; i++) {
        printf("%i, ", x[i]);
    }
    printf("\n");
}

int main() {
    init_gpio();

    int* old_status = query_all();
    int* current_status;
    
    while(1) {
        current_status = query_all();

        if (memcmp(old_status, current_status, NB_PINS*sizeof(int))!=0) {
            // TBD: Logging to file
            print_array(current_status, NB_PINS);

            // Save current state as new default to compare status against
            free(old_status);
            old_status = query_all();

        }

        free(current_status);
        delay(100);
    }
    return 0;
}
