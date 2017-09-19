#include "draw.h"
#include <stdbool.h>
#include <stdlib.h>

#include "ssd1306.h"
#include "font.c42"


void draw_char(const uint8_t x, const uint8_t page, const char c) {
  uint8_t i;
  for (i = 0; i < 5; i++ ) {
    ssd1306_write_byte(x + i, page, *(font+(c*5)+i));
  }
}


void draw_str(uint8_t x, uint8_t page, const char* s) {
  while (s && *s) {
    draw_char(x, page, *s++);
    x += 6;     // 6 pixels wide
    if (x + 6 >= SSD1306_X_PIXELS) {
      x = 0;    // ran out of this page
      page++;
    }
    if (page >= SSD1306_PIXEL_PAGES)
      break;    // ran out of space :(
  }
}


#include <stdio.h>
#include <stdarg.h>

void draw_printf(uint8_t x, const uint8_t p, const char* format, ...) {
  char dest[22 /* (128/6) + 1 = chars per line */];
  va_list argptr;
  va_start(argptr, format);
  vsnprintf(dest, sizeof dest, format, argptr);
  va_end(argptr);
  draw_str(x, p, dest);
}


void draw_clr() {
  ssd1306_clear_screen();
}


void draw_init() {
  ssd1306_init_display();
}
