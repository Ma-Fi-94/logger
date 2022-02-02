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

void log_state(int* x, int n) {
    // Trying to open logfile
    FILE *f = fopen(LOGFILE, "a");
    
    if (f == NULL) {
        perror("Error opening logfile");
        exit(-1);
    } else {
        
        // Trying to write timestamp to logfile
        time_t t;
        time(&t);
        if( fprintf(f, "%s", ctime(&t)) <= 0 ) {
            perror("Error writing to logfile");
        }
        
        // Trying to write GPIO states to logfile
        for (int i = 0; i < n; i++) {
            if ( fprintf(f, ",%i", x[i]) <= 0 ) {
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
    }
}


int main() {
    init_gpio();
    int* old_status = query_all();
    int* current_status;
    
    while(1) {
        current_status = query_all();

        if (memcmp(old_status, current_status, NB_PINS*sizeof(int))!=0) {
            // Write new state to screen
            print_array(current_status, NB_PINS);
            
            // Write new status to logfile
            log_state(current_status, NB_PINS);

            // Save current state as new default to compare status against
            free(old_status);
            old_status = query_all();

        }

        free(current_status);
        delay(POLLING_INTERVAL_MS);
    }
    return 0;
}
