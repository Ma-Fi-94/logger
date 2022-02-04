// The interval [ms] after an event, where no
// new event in the same pin is logged.
// This is to avoid logging contact bounce
#define WAIT_INTERVAL_MS 20

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

struct timedstate {
        int state;
        time_t time_of_state;
};

// We only ever use these two arrays to avoid repeated malloc() and free().
// Thus, we avoid heap fragmentation during longer use.
// We make them global to avoid passing them around.
// This should give a slight performance gain, I guess.
struct timedstate* LAST_STATE_WITH_TIME;
int* CURRENT_STATE;

void record_last_state() {
        /*
         * Record current state and time to LAST_STATE_WITH_TIME.
         */
        for (int i = 0; i < NB_PINS; i++) {
                LAST_STATE_WITH_TIME[i].state = digitalRead(PINS[i]);
                LAST_STATE_WITH_TIME[i].time_of_state = time(NULL);
        }
}


void record_current() {
        /*
         * Query all desired GPIO pins and write status to CURRENT_STATE
         */

        for (int i = 0; i < NB_PINS; i++) {
            CURRENT_STATE[i] = digitalRead(PINS[i]);
        }
}


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

void print_current() {
        /*
         * Helper function to print the CURRENT_STATE
         */

        for (int i = 0; i < NB_PINS; i++) {
                printf("%i, ", CURRENT_STATE[i]);
        }
        printf("\n");
}

void handle() {
    /*
     * Called when one of the tracked pins changes
     */

    // Query current state
    record_current();

    // TODO to avoid contact bounce:
    // We now need to figure out, which pin has changed by comparing CURRENT_STATE with LAST_STATE_WITH_TIME
    // Then, we need to check if the last event of this pin was within WAIT_INTERVAL_MS
    // If yes, we just leave right here. If not, we continue.

    // Print it to screen
    // TODO: Add debug switch for this
    print_current(CURRENT_STATE);

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
            if ( fprintf(f, ",%i", CURRENT_STATE[i]) <= 0 ) {
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
        // Init pins
        init_gpio();

        // Allocate memory for the two globals
        LAST_STATE_WITH_TIME  = (struct timedstate*) malloc(NB_PINS * sizeof(struct timedstate));
        CURRENT_STATE  = (int*) malloc(NB_PINS * sizeof(int));
        if (LAST_STATE_WITH_TIME == NULL || CURRENT_STATE == NULL) {
                perror("Error allocating memory");
                exit(-1);
        }

        // Set up last_state with the current pin states and current time
        record_last_state();

        // Bind interrupt to handler
        for (int i = 0; i < NB_PINS; i++) {
                wiringPiISR(PINS[i], INT_EDGE_BOTH, &handle);
        }

        // Waste time
        while (1) {
                delay(1);
        }


}
