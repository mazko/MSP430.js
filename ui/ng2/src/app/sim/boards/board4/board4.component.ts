import { Component, OnInit, OnDestroy } from '@angular/core';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher, IUICatcher } from '../decorators';
import { MspsimService } from '../../services/mspsim.service';
import { ControllerService } from '../../services/controller.service';
import { StateService } from '../../services/state.service';


function bit_test(port: number, bit: number): boolean {
  /* tslint:disable:no-bitwise */
  return !!(port && ((1 << bit) & port));
  /* tslint:enable:no-bitwise */
}


@Component({
  selector: 'app-board4',
  templateUrl: '../board1/board1.component.html',
  styleUrls: ['../board1/board1.component.css'],
  providers: [MspsimService],
})
export class Board4Component implements OnInit, OnDestroy, IUICatcher {

  private readonly _subscriptions: Subscription[] = [];

  private _port_cache: number[];


  get RGB_1(): string {
    return this.rgb_bit_mask(0);
  }
  get RGB_2(): string {
    return this.rgb_bit_mask(1);
  }
  get RGB_3(): string {
    return this.rgb_bit_mask(2);
  }
  get RGB_4(): string {
    return this.rgb_bit_mask(3);
  }
  get RGB_5(): string {
    return this.rgb_bit_mask(4);
  }
  get RGB_6(): string {
    return this.rgb_bit_mask(5);
  }
  get RGB_7(): string {
    return this.rgb_bit_mask(6);
  }
  get RGB_8(): string {
    return this.rgb_bit_mask(7);
  }

  get RGB_9(): string {
    return this.rgb_bit_mask(0, 1);
  }
  get RGB_10(): string {
    return this.rgb_bit_mask(1, 1);
  }
  get RGB_11(): string {
    return this.rgb_bit_mask(2, 1);
  }
  get RGB_12(): string {
    return this.rgb_bit_mask(3, 1);
  }
  get RGB_13(): string {
    return this.rgb_bit_mask(4, 1);
  }
  get RGB_14(): string {
    return this.rgb_bit_mask(5, 1);
  }
  get RGB_15(): string {
    return this.rgb_bit_mask(6, 1);
  }
  get RGB_16(): string {
    return this.rgb_bit_mask(7, 1);
  }

  private rgb_bit_mask(bit, offset = 0) {
    const cache = this._port_cache, r = cache[offset], g = cache[offset + 2], b = cache[offset + 4];
    return `rgb(${bit_test(r, bit) ? 255 : 0},${bit_test(g, bit) ? 255 : 0},${bit_test(b, bit) ? 255 : 0})`;
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


  ngOnInit(): void {
    this._state.currentBoardId = 4;
    this._subscriptions.push(this._state
      .subscribe('simulation', (): void => {
        if (!this._state.isRunning) {
          this._clear_leds();
        }
      })
    );
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
          // P1 => this._port_cache[0]
          const port = data.id - 1;
          assert(port >= 0 && port < cache.length && Number(data.value) === data.value,
                  new Error('Port ? ' + port));
          cache[port] = data.value;

          break;

        default:
          assert(false, new Error('event ? ' + event));
      }
    };

    this._sim.start('proiot4', onDataEvent);
  }


  private _clear_leds(): void {
    this._port_cache = Array<number>(6).fill(0);
  }

}
