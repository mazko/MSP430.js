import { Component, OnInit } from '@angular/core';
import { AbstractBoard } from '../AbstractBoard';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher } from '../decorators';


const DEFAULT_RGB = '#000';


@Component({
  selector: 'app-board1',
  templateUrl: './board1.component.html',
  styleUrls: ['./board1.component.css']
})
export class Board1Component extends AbstractBoard implements OnInit {

  RGB_1: string;
  RGB_2: string;
  RGB_3: string;
  RGB_4: string;
  RGB_5: string;
  RGB_6: string;
  RGB_7: string;
  RGB_8: string;

  RGB_9: string;
  RGB_10: string;
  RGB_11: string;
  RGB_12: string;
  RGB_13: string;
  RGB_14: string;
  RGB_15: string;
  RGB_16: string;

  private readonly _rgb_cache = new class {

    readonly R1 = Array<number>(8).fill(0);
    readonly G1 = [...this.R1];
    readonly B1 = [...this.R1];

    readonly R2 = [...this.R1];
    readonly G2 = [...this.R1];
    readonly B2 = [...this.R1];

    n = 0;

    reset() {
      this.n = 0;
      for (const c of [this.R1, this.G1, this.B1, this.R2, this.G2, this.B2]) {
        for (let i = 0; i < c.length; i++) {
          c[i] = 0;
        }
      }
    }

    normalize(): void {
      for (const c of [this.R1, this.G1, this.B1, this.R2, this.G2, this.B2]) {
        for (let i = 0; i < c.length; i++) {
          c[i] = Math.ceil(c[i] * 0xFF / this.n);
          if (c[i] > 0xFF) {
            console.log(`ERROR: wrong math ${c[i]} !`);
            c[i] = 0xFF;
          }
        }
      }
    }

  };

  ngOnInit(): void {
    this._state.currentBoardId = 1;
    this._subscriptions.push(this._state
      .subscribe('simulation', (): void => {
        if (!this._state.isRunning) {
          this._clear_leds();
        }
      })
    );
  }

  private _clear_leds(): void {
    this.RGB_1 = this.RGB_2 = this.RGB_3 =
    this.RGB_4 = this.RGB_5 = this.RGB_6 =
    this.RGB_7 = this.RGB_8 = this.RGB_9 =
    this.RGB_10 = this.RGB_11 = this.RGB_12 =
    this.RGB_13 = this.RGB_14 = this.RGB_15 =
    this.RGB_16 = DEFAULT_RGB;
  }

  @ui_catcher
  _do_step(): number {
    const sim = this._sim, chunk = sim.step(),
          cache = this._rgb_cache;

    cache.n += chunk.sss.length;

    for (const {PORT1, PORT2, PORT3, PORT4, PORT5, PORT6} of chunk.sss) {
      for (let i = 0; i < cache.R1.length; i++) {
        if (sim.bit_test(PORT1, i)) { cache.R1[i]++; }
        if (sim.bit_test(PORT2, i)) { cache.R2[i]++; }
        if (sim.bit_test(PORT3, i)) { cache.G1[i]++; }
        if (sim.bit_test(PORT4, i)) { cache.G2[i]++; }
        if (sim.bit_test(PORT5, i)) { cache.B1[i]++; }
        if (sim.bit_test(PORT6, i)) { cache.B2[i]++; }
      }
    }

    return chunk.UPTIME_MS;
  }

  @ui_catcher
  _do_apply_steps(): void {
    const cache = this._rgb_cache;

    cache.normalize();

    this.RGB_1 = `rgb(${cache.R1[0]},${cache.G1[0]},${cache.B1[0]})`;
    this.RGB_2 = `rgb(${cache.R1[1]},${cache.G1[1]},${cache.B1[1]})`;
    this.RGB_3 = `rgb(${cache.R1[2]},${cache.G1[2]},${cache.B1[2]})`;
    this.RGB_4 = `rgb(${cache.R1[3]},${cache.G1[3]},${cache.B1[3]})`;
    this.RGB_5 = `rgb(${cache.R1[4]},${cache.G1[4]},${cache.B1[4]})`;
    this.RGB_6 = `rgb(${cache.R1[5]},${cache.G1[5]},${cache.B1[5]})`;
    this.RGB_7 = `rgb(${cache.R1[6]},${cache.G1[6]},${cache.B1[6]})`;
    this.RGB_8 = `rgb(${cache.R1[7]},${cache.G1[7]},${cache.B1[7]})`;

    this.RGB_9 = `rgb(${cache.R2[0]},${cache.G2[0]},${cache.B2[0]})`;
    this.RGB_10 = `rgb(${cache.R2[1]},${cache.G2[1]},${cache.B2[1]})`;
    this.RGB_11 = `rgb(${cache.R2[2]},${cache.G2[2]},${cache.B2[2]})`;
    this.RGB_12 = `rgb(${cache.R2[3]},${cache.G2[3]},${cache.B2[3]})`;
    this.RGB_13 = `rgb(${cache.R2[4]},${cache.G2[4]},${cache.B2[4]})`;
    this.RGB_14 = `rgb(${cache.R2[5]},${cache.G2[5]},${cache.B2[5]})`;
    this.RGB_15 = `rgb(${cache.R2[6]},${cache.G2[6]},${cache.B2[6]})`;
    this.RGB_16 = `rgb(${cache.R2[7]},${cache.G2[7]},${cache.B2[7]})`;

    cache.reset();
  }

}
