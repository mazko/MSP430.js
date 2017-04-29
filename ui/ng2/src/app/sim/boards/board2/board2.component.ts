import { Component, OnInit } from '@angular/core';
import { AbstractBoard } from '../AbstractBoard';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher } from '../decorators';
import { SSD1306 } from './parts/SSD1306';


const DEFAULT_RGB = '#000';

// TODO: make SSD1306 page (or row?) as component to reduce template size

/*

 https://developer.mozilla.org/en/docs/Web/SVG/Attribute/y
 If the attribute is not specified, the default value is 0

 https://developer.mozilla.org/ru/docs/Web/SVG/Attribute/stroke
 The default value for the stroke attribute is none

*/

/*

~$ echo '
"use strict";

const width = 85.039 - 5.315 - .22, height = 46.063 - 5.315 - 0.22,
      x = 12.402 + 5.315 / 2, y = 370.28 + 5.315 / 2,
      Y_DOTS = 64, X_DOTS = 128,
      h = (height/Y_DOTS)*5/4,
      w = (width/X_DOTS)*5/4;

console.log(`<g transform="scale(${w},${h})">`);

for (let dy = 0; dy < Y_DOTS; dy++) {

  console.log(`<g transform="translate(0,${(y+(height*dy/Y_DOTS))/h})">`);

  for (let dx = 0, k = dy*X_DOTS; dx < X_DOTS; dx++) {
    console.log(`<rect [attr.fill]="getPixel(${ k+dx })"
      x="${(x+(width*dx/X_DOTS))/w}" width="1" height="1" />`);
  }

  console.log("</g>");
}

console.log("</g>");

' | node | xclip -selection clipboard

*/


@Component({
  selector: 'app-board2',
  templateUrl: './board2.component.html',
  styleUrls: ['./board2.component.css']
})
export class Board2Component extends AbstractBoard implements OnInit {

  RGB_0: string;
  RGB_1: string;
  RGB_2: string;
  RGB_3: string;
  RGB_4: string;
  RGB_5: string;
  RGB_6: string;
  RGB_7: string;
  RGB_8: string;
  RGB_9: string;

  BTN: number;

  private readonly _rgb_cache = new class {

    readonly R = Array<number>(8).fill(0);
    readonly G = [...this.R];
    readonly B = [...this.R];

    R1 = 0;
    G1 = 0;
    B1 = 0;

    R2 = 0;
    G2 = 0;
    B2 = 0;

    n = 0;

    reset() {
      this.R1 = this.G1 = this.B1 =
      this.R2 = this.G2 = this.B2 = this.n = 0;

      for (const c of [this.R, this.G, this.B]) {
        for (let i = 0; i < c.length; i++) {
          c[i] = 0;
        }
      }
    }

    private _norm_color(c: number): number {
      const r = Math.ceil(c * 0xFF / this.n);
      if (r > 0xFF) {
        console.log(`ERROR: wrong math ${c}/${this.n} > 0xFF !`);
        return 0xFF;
      }
      return r;
    }

    normalize(): void {
      for (const c of [this.R, this.G, this.B]) {
        for (let i = 0; i < c.length; i++) {
          c[i] = this._norm_color(c[i]);
        }
      }
      this.R1 = this._norm_color(this.R1);
      this.G1 = this._norm_color(this.G1);
      this.B1 = this._norm_color(this.B1);

      this.R2 = this._norm_color(this.R2);
      this.G2 = this._norm_color(this.G2);
      this.B2 = this._norm_color(this.B2);
    }

  };

  private _ssd1306: SSD1306;

  getPixel(idx: number): string {
    if (this._ssd1306.DISPLAY_INVERT) {
      return this._ssd1306.getPixel(idx) ? '#000' : '#aaf';
    } else {
      return this._ssd1306.getPixel(idx) ? '#aaf' : '#000';
    }
  }

  get DISPLAY_OPACITY(): number {
    return this._ssd1306.DISPLAY_ON ? this._ssd1306.PIXEL_OPACITY : 0;
  }

  get TRANSFORM() {
    // http://stackoverflow.com/a/23902773
    // translate(<minX+maxX>,0) scale(-1, 1)   // for flip X
    // translate(0,<minY+maxY>) scale(1, -1)   // for flip Y

    const x = 85.039 - 5.315 + 2 * (12.402 + 5.315 / 2),
          y = 46.063 - 5.315 + 2 * (370.28 + 5.315 / 2);

    // X and Y flip
    if (this._ssd1306.DISPLAY_TRANSFORM.MIRROR_X &&
        this._ssd1306.DISPLAY_TRANSFORM.MIRROR_Y) {
      return `translate(${x}, ${y}) scale(-1, -1)`;
    }
    // X flip
    if (this._ssd1306.DISPLAY_TRANSFORM.MIRROR_X) {
      return `translate(${x}, 0) scale(-1, 1)`;
    }
    // Y flip
    if (this._ssd1306.DISPLAY_TRANSFORM.MIRROR_Y) {
      return `translate(0, ${y}) scale(1, -1)`;
    }
    // Normal display
    return null;
  }

  ngOnInit(): void {
    this._state.currentBoardId = 2;
    this._subscriptions.push(this._state
      .subscribe('simulation', (): void => {
        if (!this._state.isRunning) {
          this._ssd1306 = new SSD1306();
          this._clear_leds();
          this.btn_mouseup();
        }
      })
    );
  }

  btn_mouseup(): void {
    this.BTN = 0;
  }

  private _clear_leds(): void {
    this.RGB_0 = this.RGB_1 = this.RGB_2 =
    this.RGB_3 = this.RGB_4 = this.RGB_5 =
    this.RGB_6 = this.RGB_7 = this.RGB_8 =
    this.RGB_9 = DEFAULT_RGB;
  }

  @ui_catcher
  _do_step(): number {
    const sim = this._sim, chunk = sim.step(this.BTN),
          cache = this._rgb_cache;

    cache.n += chunk.sss.length;

    for (const { PORT1, PORT2, PORT3, PORT4, PORT5, PORT6 } of chunk.sss) {
      if (sim.bit_test(PORT3, 1) /* E */) {
        if (!this._ssd1306.E) {
          if (sim.bit_test(PORT3, 0) /* command or data */) {
            this._ssd1306.write_data(PORT2);
          } else {
            this._ssd1306.write_command(PORT2);
          }
          this._ssd1306.E = true;
        }
      } else {
        this._ssd1306.E = false;
      }

      for (let i = 0; i < cache.R.length; i++) {
        if (sim.bit_test(PORT4, i)) { cache.R[i]++; }
        if (sim.bit_test(PORT5, i)) { cache.G[i]++; }
        if (sim.bit_test(PORT6, i)) { cache.B[i]++; }
      }

      if (sim.bit_test(PORT3, 2)) { cache.R1++; }
      if (sim.bit_test(PORT3, 3)) { cache.G1++; }
      if (sim.bit_test(PORT3, 4)) { cache.B1++; }

      if (sim.bit_test(PORT3, 5)) { cache.R2++; }
      if (sim.bit_test(PORT3, 6)) { cache.G2++; }
      if (sim.bit_test(PORT3, 7)) { cache.B2++; }
    }

    return chunk.UPTIME_MS;
  }

  @ui_catcher
  _do_apply_steps(): void {
    const cache = this._rgb_cache;

    cache.normalize();

    this.RGB_0 = `rgb(${cache.R[0]},${cache.G[0]},${cache.B[0]})`;
    this.RGB_1 = `rgb(${cache.R[1]},${cache.G[1]},${cache.B[1]})`;
    this.RGB_2 = `rgb(${cache.R[2]},${cache.G[2]},${cache.B[2]})`;
    this.RGB_3 = `rgb(${cache.R[3]},${cache.G[3]},${cache.B[3]})`;
    this.RGB_4 = `rgb(${cache.R[4]},${cache.G[4]},${cache.B[4]})`;
    this.RGB_5 = `rgb(${cache.R[5]},${cache.G[5]},${cache.B[5]})`;
    this.RGB_6 = `rgb(${cache.R[6]},${cache.G[6]},${cache.B[6]})`;
    this.RGB_7 = `rgb(${cache.R[7]},${cache.G[7]},${cache.B[7]})`;

    this.RGB_8 = `rgb(${cache.R1},${cache.G1},${cache.B1})`;
    this.RGB_9 = `rgb(${cache.R2},${cache.G2},${cache.B2})`;

    cache.reset();
  }

  on_btn_1($event: Event): void {
    this.BTN = 1;
    $event.preventDefault();
  }

  on_btn_2($event: Event): void {
    this.BTN = 2;
    $event.preventDefault();
  }

  on_btn_3($event: Event): void {
    this.BTN = 4;
    $event.preventDefault();
  }

  on_btn_4($event: Event): void {
    this.BTN = 8;
    $event.preventDefault();
  }

  on_btn_5($event: Event): void {
    this.BTN = 16;
    $event.preventDefault();
  }

  on_btn_6($event: Event): void {
    this.BTN = 32;
    $event.preventDefault();
  }

  on_btn_7($event: Event): void {
    this.BTN = 64;
    $event.preventDefault();
  }

  on_btn_8($event: Event): void {
    this.BTN = 128;
    $event.preventDefault();
  }

}
