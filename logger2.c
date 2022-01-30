#include <stdio.h>
#include <wiringPi.h>

int main() {
    wiringPiSetupGpio();

    for (int i = 2; i <= 4; i++) {
        pinMode (i, INPUT);
        pullUpDnControl(i, PUD_UP);
    }

    while(1) {
        for (int i = 2; i <= 4; i++) {
            printf("%d ", digitalRead(i));
        }
        printf("\n");
        delay(100);
    }
    return 0;
}

