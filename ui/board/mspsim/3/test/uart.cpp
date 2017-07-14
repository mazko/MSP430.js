/*
reset && (export GCC_DIR=~/ti/msp430_gcc ; 
    $GCC_DIR/bin/msp430-elf-c++ \
    -I $GCC_DIR/include -L $GCC_DIR/include \
    -Werror -Wall -mmcu=msp430f1611 -O2 uart.cpp -o uart.out)
*/

#include <msp430.h>
#include <stdint.h>

static void uart_print(const char *str) {
  while (*str) {
    while (!(IFG1 & UTXIFG0)); // USART0 TX buffer ready?
    TXBUF0 = *str++;
  }
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

    // http://www.glitovsky.com/Tutorialv0_4.pdf

    P3SEL |= 0x30; // P3.4,5 = USART0 TXD/RXD
    ME1 |= UTXE0 + URXE0; // Enable USART0 TXD/RXD
    UCTL0 |= CHAR; // 8-bit character
    UTCTL0 |= SSEL0; // UCLK = ACLK = 32.768kHz
    UBR00 = 0x03; // 32.768kHz/9600 - 3.41
    UBR10 = 0x00;
    UMCTL0 = 0x4a; // Modulation
    UCTL0 &= ~SWRST; // Initialize USART state machine
    IE1 |= URXIE0; // Enable USART0 RX interrupt

    asm volatile ( "NOP" );
    _enable_interrupts();

    P3OUT = 0;
    P3DIR = 0xFF;

    uart_print("Hello proiot.ru !\n");

    P3OUT = 1 << 2;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 1 << 5;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 0;

    P3OUT = 1 << 3;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 1 << 6;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 0;

    P3OUT = 1 << 4;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 1 << 7;
    for ( volatile uint32_t delay = 0; delay < 0x1FFFF; delay++ );
    P3OUT = 0;

    for (;;) {
      uart_print("ping !\n");
    }

    return 0;
}


void __attribute__ ((interrupt(USART0RX_VECTOR))) usartRxISR1 (void) {
    while (!(IFG1 & UTXIFG0)); // USART0 TX buffer ready?
    TXBUF0 = RXBUF0; // RXBUF0 to TXBUF0
}