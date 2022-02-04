// The polling interval [ms]
#define POLLING_INTERVAL_MS 100

// The name of the logfile
#define LOGFILE "./history.log"

// The GPIO pins we want to log
int PINS[] = {2, 3, 4};

///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>

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

void query_all(int* x) {
    /*init_gpio
     *Query all desired GPIO pins and write status to parameter
     */
    
    for (int i = 0; i < NB_PINS; i++) {
        x[i] = digitalRead(PINS[i]);
    }
}

void print_state(int* x) {
    /*
     * Helper function to print the GPIO pin state
     */

    for (int i = 0; i < NB_PINS; i++) {
        printf("%i, ", x[i]);
    }
    printf("\n");
}

void handle() {
    /*
     * Called when one of the tracked pins changes
     */

    // Query current state
    int* state = (int*) malloc(NB_PINS * sizeof(int));
    query_all(state);

    // Print it to screen
    // TODO: Add debug switch for this
    print_state(state);

    // Trying to open logfile
    FILE *f = fopen(LOGFILE, "a");
    
    if (f == NULL) {
        perror("Error opening logfile");
        exit(-1);
    } else {
        
        // Trying to write timestamp to logfile
        time_t t;
        time(&t);
        struct tm *tm_ptr = localtime(&t);
        char s[100];
        strftime(s, 100, "%Y-%m-%d,%H:%M:%S", tm_ptr);
        if( fprintf(f, "%s", s) <= 0 ) {
            perror("Error writing to logfile");
        }
        
        // Trying to write GPIO states to logfile
        for (int i = 0; i < NB_PINS; i++) {
            if ( fprintf(f, ",%i", state[i]) <= 0 ) {
                perror("Error writing to logfile");
            }
        }
        if ( fprintf(f, "\n") <= 0 ) {
            perror("Error writing to logfile");
        }
        
        // Trying to close logfile
        if ( fclose(f) != 0 ) {
            perror("Error closing logfile");
            exit(-1);
        }

        // Cleanup
        free(state);
    }
}


int main() {
        // Init
        init_gpio();

        // Bind interrupt to handler
        for (int i = 0; i < NB_PINS; i++) {
                wiringPiISR(PINS[i], INT_EDGE_BOTH, &handle);
        }

        // Waste time
        while (1) {
                delay(1);
        }


}
