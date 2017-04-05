/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

https://github.com/buserror/simavr/blob/master/examples/board_ssd1306/atmega32_ssd1306.c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include "images.h"

#include "ssd1306.h"

void
demo_show_image (void)
{
  ssd1306_write_image_fb (logo);
  ssd1306_display_fb ();
  __delay_cycles(200000);
}

void
demo_set_power_state (void)
{
  ssd1306_set_power_state (POWER_STATE_SLEEP);
  __delay_cycles(200000);
  ssd1306_set_power_state (POWER_STATE_ON);
  __delay_cycles(200000);
}

void
demo_set_contrast (void) {
  for (uint16_t contrast = 0; contrast <= 255; contrast++) {
    ssd1306_set_contrast (contrast);
    __delay_cycles(1000);
  }
}

/* Draw some dots by writing directly to the VRAM */
void
demo_set_byte_direct (void)
{
  ssd1306_clear_screen ();
  uint8_t x = 0;
  for (uint8_t page = 0; page < SSD1306_PIXEL_PAGES; ++page)
  {
    for (x = 0; x < SSD1306_X_PIXELS; x += 2)
    {
      ssd1306_write_byte (x, page, 0xAA);
    }
  }
  __delay_cycles(300000);
}

/* Draw some stripes by setting individual pixels on the frame buffer */
void
demo_set_pixels (void)
{
  ssd1306_clear_fb ();
  uint8_t x = 0;
  for (uint8_t y = 0; y < SSD1306_Y_PIXELS; ++y)
  {
    for (x = 0; x < SSD1306_X_PIXELS; x += 2)
    {
      ssd1306_set_pixel_fb (x, y, PIXEL_STATE_ON);
    }
  }
  ssd1306_display_fb ();
  __delay_cycles(300000);
}

void
demo_clear_screen (void)
{
  // Turn all pixels on
  memset (ssd1306_frame_buffer_g, 0xFF, SSD1306_PIXEL_BYTES);
  ssd1306_display_fb ();
  __delay_cycles(200000);

  // Clear screen
  ssd1306_clear_screen ();
  __delay_cycles(200000);
}

void
demo_rotate_display (void)
{
  for (uint8_t i = DISP_ORIENT_NORMAL;
                  i <= DISP_ORIENT_UPSIDE_DOWN_MIRRORED; ++i)
  {
    ssd1306_set_display_orientation (i);
    ssd1306_write_image_fb (logo);
    ssd1306_display_fb ();
    __delay_cycles(200000);
  }
}


void
demo_invert_image ()
{
  ssd1306_set_display_orientation (DISP_ORIENT_NORMAL);
  for (uint8_t i = DISPLAY_MODE_NORMAL; i <= DISPLAY_MODE_INVERTED; ++i)
  {
    ssd1306_set_display_mode (i);
    ssd1306_write_image_fb (logo);
    ssd1306_display_fb ();
    // Check inverted contrast works
    demo_set_contrast ();
  }
}


void rotate_rgb(volatile unsigned char * const port) {
    volatile unsigned int i;            // volatile to prevent optimization

    *port = 1;
    do {
        __delay_cycles(100000);
        *port <<= 1;
    } while (*port);
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

    // DCO = 3, RSEL = 0, f = 0.13 MHz
    DCOCTL = /* DCO2 + */ DCO1 + DCO0; 
    BCSCTL1 = XT2OFF /* + RSEL1 + RSEL0 + RSEL2 */;

    const uint8_t O = 0;

    P2OUT = O; P3OUT = O; P4OUT = O; P5OUT = O; P6OUT = O;
    P2DIR = P3DIR = P4DIR = P5DIR = P6DIR = 0xFF;

    for (int i = 0; i < 8; i++) {
      while (!P1IN);
      P4OUT = P1IN;
      while (P1IN);
    }

    // test out of bounds should print console.log  
    ssd1306_write_byte(128, 8, 0);

    for(;;) {

      // leds
      rotate_rgb(&P4OUT);
      P3OUT = 1 << 2;
      __delay_cycles(100000);
      P3OUT = 1 << 5;
      __delay_cycles(100000);
      P3OUT = 0;
      rotate_rgb(&P5OUT);
      P3OUT = 1 << 3;
      __delay_cycles(100000);
      P3OUT = 1 << 6;
      __delay_cycles(100000);
      P3OUT = 0;
      rotate_rgb(&P6OUT);
      P3OUT = 1 << 4;
      __delay_cycles(100000);
      P3OUT = 1 << 7;
      __delay_cycles(100000);
      P3OUT = 0;

      // display
      ssd1306_init_display();

      demo_show_image ();
      demo_set_power_state();
      demo_set_contrast();
      demo_set_byte_direct ();
      demo_set_pixels ();
      demo_clear_screen();
      demo_rotate_display ();
      demo_invert_image ();

      // for (uint8_t i = 0; i < 128; i++) {
      //   for (uint8_t j = 0; j < 8; j++) {
      //     ssd1306_write_data(0xFF);
      //   }
      //   __delay_cycles(10000);
      // }

      // for (uint8_t i = 0; i < 128; i++) {
      //   for (uint8_t j = 0; j < 8; j++) {
      //     ssd1306_write_data(0);
      //   }
      //   __delay_cycles(10000);
      // }

      // ssd1306_write_byte(42, 0, 1);
      // ssd1306_write_byte(43, 0, 2);
      // ssd1306_write_byte(44, 0, 4);
      // ssd1306_write_byte(45, 0, 8);
      // ssd1306_write_byte(46, 0, 16);
      // ssd1306_write_byte(47, 0, 32);
      // ssd1306_write_byte(48, 0, 64);
      // ssd1306_write_byte(49, 0, 128);

      // ssd1306_write_byte(50, 0, 128);
      // ssd1306_write_byte(51, 0, 64);
      // ssd1306_write_byte(52, 0, 32);
      // ssd1306_write_byte(53, 0, 16);
      // ssd1306_write_byte(54, 0, 8);
      // ssd1306_write_byte(55, 0, 4);
      // ssd1306_write_byte(56, 0, 2);
      // ssd1306_write_byte(57, 0, 1);
    }

    return 0;
}