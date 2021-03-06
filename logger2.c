// The interval [ms] after an event, where no
// new event in the same pin is logged.
// This is to avoid logging contact bounce
#define WAIT_INTERVAL_MS 20

// The name of the logfile
#define LOGFILE "./history.log"

// The GPIO pins we want to log
int PINS[] = {2, 3, 4};

// Whether to log to screen as well
#define PRINT_TO_SCREEN

///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <wiringPi.h>

int NB_PINS = (int) ((sizeof(PINS)) / (sizeof(PINS[0])));

struct timedstate {
        int state;
        time_t time_of_state_secs;
        int additional_usecs;
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
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);

        for (int i = 0; i < NB_PINS; i++) {
                LAST_STATE_WITH_TIME[i].state = digitalRead(PINS[i]);
                LAST_STATE_WITH_TIME[i].time_of_state_secs = tv_now.tv_sec;
                LAST_STATE_WITH_TIME[i].additional_usecs = tv_now.tv_usec;
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

    // Which pin has changed?
    int i_changed = -1;
    for (int i = 0; i < NB_PINS; i++) {
        if (CURRENT_STATE[i] != LAST_STATE_WITH_TIME[i].state) {
                i_changed = i;
                break;
        }
    }

    // If nothing has changed (contact bounce), we leave
    if (i_changed == -1) {
        return;
    }

    // Was the change within WAIT_INTERVAL_MS?
    // If yes, we ignore it (contact bounce) and leave
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    double time_diff_ms = difftime(tv_now.tv_sec, LAST_STATE_WITH_TIME[i_changed].time_of_state_secs) * 1000.0;
    time_diff_ms += (tv_now.tv_usec - LAST_STATE_WITH_TIME[i_changed].additional_usecs) / 1000.0; 
    if (time_diff_ms < WAIT_INTERVAL_MS) {
        return;
    }

    // Print current state to screen if desired
    // To avoid an if every time we're here, we use the preprocessor
    #ifdef PRINT_TO_SCREEN
    print_current();
    #endif

    // Trying to open logfile
    FILE *f = fopen(LOGFILE, "a");

    if (f == NULL) {
        perror("Error opening logfile");
        exit(-1);
    }


    // Trying to write timestamp to logfile
    time_t t;
    time(&t);
    struct tm *tm_ptr = localtime(&t);
    char s[100];
    strftime(s, 100, "%Y-%m-%d,%H:%M:%S", tm_ptr);
    if( fprintf(f, "%s", s) <= 0 ) {
        perror("Error writing to logfile");
        exit(-1);
    }

    // Trying to write GPIO states to logfile
    for (int i = 0; i < NB_PINS; i++) {
        if ( fprintf(f, ",%i", CURRENT_STATE[i]) <= 0 ) {
            perror("Error writing to logfile");
            exit(-1);
        }
    }
    if ( fprintf(f, "\n") <= 0 ) {
        perror("Error writing to logfile");
        exit(-1);
    }

    // Trying to close logfile
    if ( fclose(f) != 0 ) {
        perror("Error closing logfile");
        exit(-1);
    }

    // Save current to last_state
    record_last_state();
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
