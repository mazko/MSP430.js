/*
 ssd1306.c

 SSD1306 display driver (SPI mode)

 Copyright 2014 Doug Szumski <d.s.szumski@gmail.com>

 Inspired by the work of Gabriel Anzziani.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <string.h>
#include <msp430.h>


#include "ssd1306.h"



enum { SSD1306_DATA_INST = 0, SSD1306_DATA_E = 1 };

static void strobe(void) {
  P3OUT |= (1 << SSD1306_DATA_E);
  P3OUT &= ~(1 << SSD1306_DATA_E);
}

void
ssd1306_write_data (const uint8_t byte) {
  P2OUT = byte;
  P3OUT |= (1 << SSD1306_DATA_INST);
  strobe();
}

void
ssd1306_write_instruction (const uint8_t byte) {
  P2OUT = byte;
  P3OUT &= ~(1 << SSD1306_DATA_INST);
  strobe();
}

/* Initialise display mostly as per p64 of the datasheet */
void
ssd1306_init_display (void)
{

  ssd1306_set_power_state (POWER_STATE_SLEEP);

  ssd1306_write_instruction (SSD1306_SET_MULTIPLEX_RATIO);
  ssd1306_write_instruction (0x3F);

  ssd1306_write_instruction (SSD1306_SET_VERTICAL_OFFSET);
  ssd1306_write_instruction (0x00);

  ssd1306_write_instruction (SSD1306_SET_DISP_START_LINE);

  ssd1306_set_display_orientation(DISP_ORIENT_NORMAL);

  ssd1306_write_instruction (SSD1306_SET_WIRING_SCHEME);
  ssd1306_write_instruction (0x12);

  ssd1306_set_contrast (SSD1306_DEFAULT_CONTRAST);

  ssd1306_write_instruction (SSD1306_RESUME_TO_RAM_CONTENT);

  ssd1306_set_display_mode (DISPLAY_MODE_NORMAL);

  // Horizontal memory addressing mode
  ssd1306_write_instruction (SSD1306_MEM_ADDRESSING);
  ssd1306_write_instruction (0x00);

  ssd1306_write_instruction (SSD1306_SET_DISP_CLOCK);
  ssd1306_write_instruction (0x80);

  ssd1306_write_instruction (SSD1306_CHARGE_PUMP_REGULATOR);
  ssd1306_write_instruction (SSD1306_CHARGE_PUMP_ON);

  ssd1306_set_power_state (POWER_STATE_ON);
}

void
ssd1306_set_display_orientation (const disp_orient_t disp_orient)
{
  switch (disp_orient)
    {
    case DISP_ORIENT_NORMAL:
      ssd1306_write_instruction (SSD1306_SET_SEG_REMAP_0);
      ssd1306_write_instruction (SSD1306_SET_COM_SCAN_NORMAL);
      break;
    case DISP_ORIENT_NORMAL_MIRRORED:
      // The display is mirrored from the upper edge
      ssd1306_write_instruction (SSD1306_SET_SEG_REMAP_0);
      ssd1306_write_instruction (SSD1306_SET_COM_SCAN_INVERTED);
      break;
    case DISP_ORIENT_UPSIDE_DOWN:
      ssd1306_write_instruction (SSD1306_SET_SEG_REMAP_127);
      ssd1306_write_instruction (SSD1306_SET_COM_SCAN_INVERTED);
      break;
    case DISP_ORIENT_UPSIDE_DOWN_MIRRORED:
      // The upside down display is mirrored from the upper edge
      ssd1306_write_instruction (SSD1306_SET_SEG_REMAP_127);
      ssd1306_write_instruction (SSD1306_SET_COM_SCAN_NORMAL);
      break;
    default:
      break;
    }
}

/* Move the cursor to the start */
void
ssd1306_reset_cursor (void)
{
  ssd1306_write_instruction (SSD1306_SET_PAGE_START_ADDR);
  ssd1306_write_instruction (SSD1306_SET_COL_HI_NIBBLE);
  ssd1306_write_instruction (SSD1306_SET_COL_LO_NIBBLE);
}

void
ssd1306_set_contrast (const uint8_t contrast)
{
  ssd1306_write_instruction (SSD1306_SET_CONTRAST);
  ssd1306_write_instruction (contrast);
}

void
ssd1306_set_display_mode(const display_mode_t display_mode)
{
  switch (display_mode) {
    case DISPLAY_MODE_NORMAL:
      ssd1306_write_instruction (SSD1306_DISP_NORMAL);
      break;
    case DISPLAY_MODE_INVERTED:
      ssd1306_write_instruction (SSD1306_DISP_INVERTED);
      break;
    default:
      ssd1306_write_instruction (SSD1306_DISP_NORMAL);
      break;
  }
}

void
ssd1306_set_power_state (const power_state_t power_state)
{
  switch (power_state)
    {
    case POWER_STATE_ON:
      ssd1306_write_instruction (SSD1306_DISP_ON);
      break;
    case POWER_STATE_SLEEP:
      ssd1306_write_instruction (SSD1306_DISP_SLEEP);
      break;
    default:
      break;
    }
}

void
ssd1306_write_byte (const uint8_t x, const uint8_t page, const uint8_t byte)
{
  ssd1306_write_instruction (SSD1306_SET_PAGE_START_ADDR | page);
  ssd1306_write_instruction (SSD1306_SET_COL_LO_NIBBLE | (x & 0xF));
  ssd1306_write_instruction (SSD1306_SET_COL_HI_NIBBLE | (x >> 4));
  ssd1306_write_data(byte);
}

void
ssd1306_clear_screen (void)
{
  ssd1306_reset_cursor ();
  uint16_t byte;
  for (byte = 0; byte < SSD1306_PIXEL_BYTES; byte++)
    {
      ssd1306_write_data (0x00);
    }
}


