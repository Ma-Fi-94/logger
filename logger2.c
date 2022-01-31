#include <stdio.h>
#include <wiringPi.h>

// The GPIO pins we want to log
int PINS[] = {2, 3, 4};
int NB_PINS = (int) ((sizeof(PINS)) / (sizeof(PINS[0])))

void init() {
    /*
     * Setup all desired GPIO pins for use as input
     * and pull them to UP
     */
    
    // Setup GPIO usage
    wiringPiSetupGpio();

    // Set all desired GPIO pins to input mode and UP
    for (int i = 0; i <= NB_PINS; i++) {
        pinMode (PINS[i], INPUT);
        pullUpDnControl(PINS[i], PUD_UP);
    }
}

int* query_all() {
    /*
     *Query all desired GPIO pins and return their status.
     */
    
    int* current_status = (int*) malloc(NB_PINS * sizeof(int));
    
    for (int i = 0; i < NB_PINS; i++) {
        current_status[i] = digitalRead(PINS[i]);
    }
    
    return (current_status);
}

int array_eq(int* a, int* b, int n) {
    /*
     * Element-wise comparison of two int array of length n.
     * Returns 1 if all elements are equal, otherwise returns 0;
     */
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}


int main() {
    int* old_status = query_all();
    int* current_status;
    
    while(1) {
        current_status = query_all();
        if (!array_eq(old_status, current_status, NB_PINS)) {
            // At least one pins has changed.
            // TBD
        }

        delay(100);
    }
    return 0;
}

