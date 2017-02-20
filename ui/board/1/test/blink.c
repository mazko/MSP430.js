// ./wsim-wsn430 --logfile=stdout --verbose=6 --realtime --ui /home/oleg/Desktop/wsim.js/wsim/blink/msp430f1611.out

#include <msp430.h>

void rotate(volatile unsigned char * const port) {
    volatile unsigned int i;            // volatile to prevent optimization

    *port = 1;
    do {
        i = 333;                          // SW Delay
        do i--;
        while(i != 0);
        *port <<= 1;
    } while (*port);
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

//BCSCTL1= 0;
    DCOCTL = 0;//0xE0; // sets DCO to max frequency
BCSCTL1 = 0;
BCSCTL2 = DIVM_3;
//DCOCTL = 0;

    //P1OUT = P2OUT = P3OUT = P4OUT = P5OUT = P6OUT = 0;
    P1DIR = P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0xFF;                          // Set P1.0 to output direction

    for(;;) {
        rotate(&P1OUT);
        rotate(&P2OUT);
        rotate(&P3OUT);
        rotate(&P4OUT);
        rotate(&P5OUT);
        rotate(&P6OUT);
    }

    return 0;
}