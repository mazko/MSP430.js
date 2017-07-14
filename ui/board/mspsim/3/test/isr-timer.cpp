/*
reset && (export GCC_DIR=~/ti/msp430_gcc ; 
    $GCC_DIR/bin/msp430-elf-c++ \
    -I $GCC_DIR/include -L $GCC_DIR/include \
    -Werror -Wall -mmcu=msp430f1611 -O2 isr-timer.cpp -o isr-timer.out)
*/

#include <msp430.h>
#include <stdint.h>

// example of timer for snake game
static void prvSetupTimerInterrupt( void ) {
    CCTL0 = CCIE;
    TACCR0 = 0xFFFF; // mspsim ignores this value
    TACTL = TASSEL_2 + MC_2; // Set the timer A to SMCLCK, Continuous
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

    // DCO = 7, RSEL = 7, f = 4.5 MHz
    DCOCTL = DCO2 + DCO1 + DCO0; 
    BCSCTL1 = XT2OFF + RSEL0 + RSEL1 + RSEL2;

    constexpr auto O = 0;

    P1OUT = O; P2OUT = O; /*P3OUT = O*/; P4OUT = O; P5OUT = O; P6OUT = O;
    P1DIR = P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0xFF;

    prvSetupTimerInterrupt();
    asm volatile ( "NOP" );
    _enable_interrupts();

    for (;;) {
        // for (volatile uint16_t i = 0; --i; );
        // P5OUT = 0xF0;
        // for (volatile uint16_t i = 0; --i; );
        // P5OUT = 0x0F;
        // for (volatile uint16_t i = 0; --i; );
        // P5OUT = 0xFF;
        // for (volatile uint16_t i = 0; --i; );
        // P5OUT = 0;
    }

    return 0;
}


void __attribute__ ((interrupt(TIMERA0_VECTOR))) prvTickISR1 (void) {
    P4OUT ^= 0xFF;
}