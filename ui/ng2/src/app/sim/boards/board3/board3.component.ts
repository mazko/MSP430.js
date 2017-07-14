import { Component, OnInit, OnDestroy } from '@angular/core';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher, IUICatcher } from '../decorators';
import { SSD1306 } from '../board2/parts/SSD1306';
import { MspsimService } from '../../services/mspsim.service';
import { ControllerService } from '../../services/controller.service';
import { StateService } from '../../services/state.service';


function bit_test(port: number, bit: number): boolean {
  /* tslint:disable:no-bitwise */
  return !!(port && ((1 << bit) & port));
  /* tslint:enable:no-bitwise */
}


@Component({
  selector: 'app-board3',
  templateUrl: '../board2/board2.component.html',
  styleUrls: ['../board2/board2.component.css'],
  providers: [MspsimService],
})
export class Board3Component implements OnInit, OnDestroy, IUICatcher {

  private readonly _subscriptions: Subscription[] = [];

  private _port_cache: number[];

  private _ssd1306: SSD1306;

  private _last_pushed_button_id = -1;

  get RGB_0(): string {
    return this.rgb_bit_mask(0);
  }
  get RGB_1(): string {
    return this.rgb_bit_mask(1);
  }
  get RGB_2(): string {
    return this.rgb_bit_mask(2);
  }
  get RGB_3(): string {
    return this.rgb_bit_mask(3);
  }
  get RGB_4(): string {
    return this.rgb_bit_mask(4);
  }
  get RGB_5(): string {
    return this.rgb_bit_mask(5);
  }
  get RGB_6(): string {
    return this.rgb_bit_mask(6);
  }
  get RGB_7(): string {
    return this.rgb_bit_mask(7);
  }

  get RGB_8(): string {
    return this.rgb_bit_mask(2, 3, 4, 0, 0, 0);
  }

  get RGB_9(): string {
    return this.rgb_bit_mask(5, 6, 7, 0, 0, 0);
  }

  private rgb_bit_mask(bit_r: number, bit_g = bit_r, bit_b = bit_r, idx_r = 1, idx_g = 2, idx_b = 3) {
    const cache = this._port_cache, r = cache[idx_r], g = cache[idx_g], b = cache[idx_b];
    return `rgb(${bit_test(r, bit_r) ? 255 : 0},${bit_test(g, bit_g) ? 255 : 0},${bit_test(b, bit_b) ? 255 : 0})`;
  }

  constructor(
      private readonly _sim: MspsimService,
      private readonly _state: StateService,
      readonly _controller: ControllerService) {

    this._subscriptions.push(_controller
      .simulationStarted$
      .subscribe(this.start.bind(this))
    );

    this._subscriptions.push(_controller
      .simulationStopped$
      .subscribe(this.stop.bind(this))
    );

    // health always OK
    this._controller.healthChanged(true);
  }


  ngOnDestroy(): void {
    this.stop();
    // prevent memory leak when component destroyed
    for (const sub of this._subscriptions) {
      sub.unsubscribe();
    }
  }


  @ui_catcher
  private stop(): void {
    this._sim.stop();
  }


  @ui_catcher
  private start(): void {
    const onDataEvent = (event: string, data: any) => {

      const assert = this._controller.assert;

      switch (event) {

        case 'port':
          const cache = this._port_cache;
          // P3 => this._port_cache[0]
          const port = data.id - 3;
          assert(port >= 0 && port < cache.length && Number(data.value) === data.value,
                  new Error('Port ? ' + port));
          cache[port] = data.value;

          break;

        case 'ssd1306':
          assert(Number(data.value) === data.value, new Error('ssd1306 value ? ' + data.value));

          if (data.command === true) {
            this._ssd1306.write_command(data.value);
          } else if (data.command === false) {
            this._ssd1306.write_data(data.value);
          } else {
            assert(false, new Error('ssd1306 command ? ' + data.command));
          }

          break;

        default:
          assert(false, new Error('event ? ' + event));
      }
    };

    this._sim.start('proiot3', onDataEvent);
  }


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
    this._state.currentBoardId = 3;
    this._subscriptions.push(this._state
      .subscribe('simulation', (): void => {
        if (!this._state.isRunning) {
          this._ssd1306 = new SSD1306();
          this._clear_leds();
        }
      })
    );
  }

  btn_mouseup(): void {
    if (this._last_pushed_button_id >= 0) {
      this._sim.button(this._last_pushed_button_id, false);
    }
  }

  private _clear_leds(): void {
    this._port_cache = Array<number>(4).fill(0);
  }

  private _button_impl(id: number, $event: Event) {
    this._last_pushed_button_id = id;
    this._sim.button(id);
    $event.preventDefault();
  }

  on_btn_1($event: Event): void {
    this._button_impl(0, $event);
  }

  on_btn_2($event: Event): void {
    this._button_impl(1, $event);
  }

  on_btn_3($event: Event): void {
    this._button_impl(2, $event);
  }

  on_btn_4($event: Event): void {
    this._button_impl(3, $event);
  }

  on_btn_5($event: Event): void {
    this._button_impl(4, $event);
  }

  on_btn_6($event: Event): void {
    this._button_impl(5, $event);
  }

  on_btn_7($event: Event): void {
    this._button_impl(6, $event);
  }

  on_btn_8($event: Event): void {
    this._button_impl(7, $event);
  }

}
