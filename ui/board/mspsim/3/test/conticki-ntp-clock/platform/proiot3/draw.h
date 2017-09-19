#include "stdint.h"

void draw_char(const uint8_t x, const uint8_t page, const char c);
void draw_str(uint8_t x, uint8_t page, const char*);
void draw_str_hex(const uint8_t, const char* const, const uint16_t);
void draw_printf(uint8_t x, const uint8_t p, const char* format, ...);

void draw_clr(void);
void draw_init(void);

