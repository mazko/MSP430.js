const SSD1306_VIRT_BITS     = 8;
const SSD1306_VIRT_PAGES    = 8;
const SSD1306_VIRT_COLUMNS  = 128;


/* Fundamental commands. */
const SSD1306_VIRT_SET_CONTRAST          = 0x81;
const SSD1306_VIRT_RESUME_TO_RAM_CONTENT = 0xA4;
const SSD1306_VIRT_IGNORE_RAM_CONTENT    = 0xA5;
const SSD1306_VIRT_DISP_NORMAL           = 0xA6;
const SSD1306_VIRT_DISP_INVERTED         = 0xA7;
const SSD1306_VIRT_DISP_SUSPEND          = 0xAE;
const SSD1306_VIRT_DISP_ON               = 0xAF;

/* Scrolling commands */
const SSD1306_VIRT_SCROLL_RIGHT  = 0x26;
const SSD1306_VIRT_SCROLL_LEFT   = 0x27;
const SSD1306_VIRT_SCROLL_VR     = 0x29;
const SSD1306_VIRT_SCROLL_VL     = 0x2A;
const SSD1306_VIRT_SCROLL_OFF    = 0x2E;
const SSD1306_VIRT_SCROLL_ON     = 0x2F;
const SSD1306_VIRT_VERT_SCROLL_A = 0xA3;

/* Address setting commands */
const SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE  = 0x00;
const SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE = 0x10;
const SSD1306_VIRT_MEM_ADDRESSING         = 0x20;
const SSD1306_VIRT_SET_COL_ADDR           = 0x21;
const SSD1306_VIRT_SET_PAGE_ADDR          = 0x22;
const SSD1306_VIRT_SET_PAGE_START_ADDR    = 0xB0;

/* Hardware config. commands */
const SSD1306_VIRT_SET_LINE              = 0x40;
const SSD1306_VIRT_SET_SEG_REMAP_0       = 0xA0;
const SSD1306_VIRT_SET_SEG_REMAP_127     = 0xA1;
const SSD1306_VIRT_MULTIPLEX             = 0xA8;
const SSD1306_VIRT_SET_COM_SCAN_NORMAL   = 0xC0;
const SSD1306_VIRT_SET_COM_SCAN_INVERTED = 0xC8;
const SSD1306_VIRT_SET_OFFSET            = 0xD3;
const SSD1306_VIRT_SET_PADS              = 0xDA;


export class SSD1306 {

    private readonly vram: boolean[] =
      Array(SSD1306_VIRT_PAGES * SSD1306_VIRT_BITS * SSD1306_VIRT_COLUMNS).fill(false);

    private readonly cursor = {
      page: 0,
      column: 0,
    };


    private command_register  = 0;
    private contrast_register = 0x7F;


    private readonly flags = {
      DISPLAY_INVERTED: false,
      DISP_SUSPEND:     false,
      SEG_REMAP_0:      false,
      SCAN_INVERTED:    false,
    };


    E = false;


    write_data(data: number): void {
      const cursor = this.cursor,
            j = cursor.page * SSD1306_VIRT_BITS * SSD1306_VIRT_COLUMNS + cursor.column;

      for (let i = 0; i < SSD1306_VIRT_BITS; i++) {
        /* tslint:disable:no-bitwise */
        this.vram[i * SSD1306_VIRT_COLUMNS + j] = !!(data & (1 << i));
        /* tslint:enable:no-bitwise */
      }

      // Scroll the cursor
      if (++(cursor.column) >= SSD1306_VIRT_COLUMNS) {
        cursor.column = 0;
        if (++(cursor.page) >= SSD1306_VIRT_PAGES) {
          cursor.page = 0;
        }
      }
    }


    /*
     * Called on the first command byte sent. For setting single
     * byte commands and initiating multi-byte commands.
     */
    private _update_command_register(data: number): void {
      const cursor = this.cursor;

      switch (data) {

        case SSD1306_VIRT_SET_CONTRAST:
          this.command_register = data;
          break;

        case SSD1306_VIRT_DISP_NORMAL:
          this.flags.DISPLAY_INVERTED = false;
          break;

        case SSD1306_VIRT_DISP_INVERTED:
          this.flags.DISPLAY_INVERTED = true;
          break;

        case SSD1306_VIRT_DISP_SUSPEND:
          this.flags.DISP_SUSPEND = true;
          break;

        case SSD1306_VIRT_DISP_ON:
          this.flags.DISP_SUSPEND = false;
          break;

        case SSD1306_VIRT_SET_SEG_REMAP_0:
          this.flags.SEG_REMAP_0 = false;
          break;

        case SSD1306_VIRT_SET_SEG_REMAP_127:
          this.flags.SEG_REMAP_0 = true;
          break;

        case SSD1306_VIRT_SET_COM_SCAN_NORMAL:
          this.flags.SCAN_INVERTED = false;
          break;

        case SSD1306_VIRT_SET_COM_SCAN_INVERTED:
          this.flags.SCAN_INVERTED = true;
          break;

        default:
          if (data >= SSD1306_VIRT_SET_PAGE_START_ADDR &&
              data < SSD1306_VIRT_SET_PAGE_START_ADDR + SSD1306_VIRT_PAGES) {
            const page = data - SSD1306_VIRT_SET_PAGE_START_ADDR;
            cursor.page = page;

          } else if (data >= SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE &&
              data <= SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE + 0xF) {
            const d = data - SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE;
            /* tslint:disable:no-bitwise */
            cursor.column = (cursor.column & 0xF0) | (d & 0xF);
            /* tslint:enable:no-bitwise */

          } else if (data >= SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE &&
              data <= SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE + 0xF) {
            const d = data - SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE;
            /* tslint:disable:no-bitwise */
            cursor.column = (cursor.column & 0xF) | ((d & 0xF) << 4);
            /* tslint:enable:no-bitwise */
          } else {
            console.log(`SSD1306: UNKNOWN COMMAND: 0x${data.toString(16).toUpperCase()}`);
          }

          // assert bounds
          if (cursor.column >= SSD1306_VIRT_COLUMNS || cursor.column < 0) {
            console.log(`SSD1306: column out of bounds: ${cursor.column}`);
            cursor.column = 0;
          }
          // cursor.page currently never out of bounds (just unknown command)
          if (cursor.page >= SSD1306_VIRT_PAGES || cursor.page < 0) {
            console.log(`SSD1306: page out of bounds: ${cursor.page}`);
            cursor.page = 0;
          }
      }
    }


    /*
     * Multi-byte command setting
     */
    _update_setting (data: number) {
      switch (this.command_register) {
        case SSD1306_VIRT_SET_CONTRAST:
          this.contrast_register = data;
          break;
        default:
          console.log(`SSD1306: LOGIC ERROR: MULTIBYTE COMMAND ?: ${data}`);
      }
      this.command_register = 0;
    }

    /*
     * Determines whether a new command has been sent, or
     * whether we are in the process of setting a multi-
     * byte command.
     */
    write_command(data: number): void {
      if (this.command_register) {
        // Multi-byte command setting
        this._update_setting(data);
      } else {
        // Single byte or start of multi-byte command
        this._update_command_register(data);
      }
    }


    get PIXEL_OPACITY() {
      // Typically the screen will be clearly visible even at 0 contrast
      return this.contrast_register / 512.0 + 0.5;
    }


    get DISPLAY_ON(): boolean {
      return !this.flags.DISP_SUSPEND;
    }


    get DISPLAY_TRANSFORM() {
      return {
        MIRROR_X: this.flags.SEG_REMAP_0,
        MIRROR_Y: this.flags.SCAN_INVERTED,
      };
    }


    get DISPLAY_INVERT(): boolean {
      return this.flags.DISPLAY_INVERTED;
    }


    getPixel(idx: number): boolean {
      return this.vram[idx];
    }
}
