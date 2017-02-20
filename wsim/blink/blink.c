// ./wsim-wsn430 --logfile=stdout --verbose=6 --realtime --ui /home/oleg/Desktop/wsim.js/wsim/blink/msp430f1611.out

#include <msp430.h>

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

//BCSCTL1= 0;
    DCOCTL = 0;//0xE0; // sets DCO to max frequency
BCSCTL1 = 0;
BCSCTL2 = DIVM_2;
//DCOCTL = 0;
    P5DIR |= 0xFF;                          // Set P1.0 to output direction

    for(;;) {
        volatile unsigned int i;            // volatile to prevent optimization

        P5OUT ^= 0xFF;                      // Toggle P1.0 using exclusive-OR

        i = 4444;                          // SW Delay
        do i--;
        while(i != 0);
    }

    return 0;
}