import { Component, OnInit } from '@angular/core';
import { AbstractBoard } from '../AbstractBoard';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher } from '../decorators';


const DEFAULT_RGB = '#000';


class RGBCache {
  private v = 0;
  private total = 0;

  update(v: boolean): void {
    if (v) {
      this.v++;
    }
    this.total++;
  }

  reset(): void {
    this.v = this.total = 0;
  }

  get n(): number {
    return Math.ceil(this.v * 0xFF / this.total);
  }
}


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

    readonly R1 = this.create();
    readonly G1 = this.create();
    readonly B1 = this.create();

    readonly R2 = this.create();
    readonly G2 = this.create();
    readonly B2 = this.create();

    private create(): RGBCache[] {
      return Array.apply(null, Array(8)).map( () => new RGBCache() );
    }

    reset() {
      for (const c of [this.R1, this.G1, this.B1, this.R2, this.G2, this.B2]) {
        for (let i = 0; i < c.length; i++) {
          c[i].reset();
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
    const snapshot = this._sim.step(),
          cache = this._rgb_cache;

    for (let i = 0; i < cache.R1.length; i++) {
      cache.R1[i].update(snapshot.get_port1_bit(i));
      cache.R2[i].update(snapshot.get_port2_bit(i));
      cache.G1[i].update(snapshot.get_port3_bit(i));
      cache.G2[i].update(snapshot.get_port4_bit(i));
      cache.B1[i].update(snapshot.get_port5_bit(i));
      cache.B2[i].update(snapshot.get_port6_bit(i));
    }

    return snapshot.UPTIME_MS;
  }

  @ui_catcher
  _do_apply_steps(): void {
    const cache = this._rgb_cache;

    this.RGB_1 = `rgb(${cache.R1[0].n},${cache.G1[0].n},${cache.B1[0].n})`;
    this.RGB_2 = `rgb(${cache.R1[1].n},${cache.G1[1].n},${cache.B1[1].n})`;
    this.RGB_3 = `rgb(${cache.R1[2].n},${cache.G1[2].n},${cache.B1[2].n})`;
    this.RGB_4 = `rgb(${cache.R1[3].n},${cache.G1[3].n},${cache.B1[3].n})`;
    this.RGB_5 = `rgb(${cache.R1[4].n},${cache.G1[4].n},${cache.B1[4].n})`;
    this.RGB_6 = `rgb(${cache.R1[5].n},${cache.G1[5].n},${cache.B1[5].n})`;
    this.RGB_7 = `rgb(${cache.R1[6].n},${cache.G1[6].n},${cache.B1[6].n})`;
    this.RGB_8 = `rgb(${cache.R1[7].n},${cache.G1[7].n},${cache.B1[7].n})`;

    this.RGB_9 = `rgb(${cache.R2[0].n},${cache.G2[0].n},${cache.B2[0].n})`;
    this.RGB_10 = `rgb(${cache.R2[1].n},${cache.G2[1].n},${cache.B2[1].n})`;
    this.RGB_11 = `rgb(${cache.R2[2].n},${cache.G2[2].n},${cache.B2[2].n})`;
    this.RGB_12 = `rgb(${cache.R2[3].n},${cache.G2[3].n},${cache.B2[3].n})`;
    this.RGB_13 = `rgb(${cache.R2[4].n},${cache.G2[4].n},${cache.B2[4].n})`;
    this.RGB_14 = `rgb(${cache.R2[5].n},${cache.G2[5].n},${cache.B2[5].n})`;
    this.RGB_15 = `rgb(${cache.R2[6].n},${cache.G2[6].n},${cache.B2[6].n})`;
    this.RGB_16 = `rgb(${cache.R2[7].n},${cache.G2[7].n},${cache.B2[7].n})`;

    this._rgb_cache.reset();
  }

}
