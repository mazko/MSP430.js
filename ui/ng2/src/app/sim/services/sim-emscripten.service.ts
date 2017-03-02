import { Injectable } from '@angular/core';

// https://angular.io/docs/ts/latest/guide/dependency-injection.html#singleton-services

const EM_SIM_MODULE = window['msp430_sim_module'];

/* tslint:disable:interface-over-type-literal */
type StepSnapshot_t = {
  readonly PORT1: number;
  readonly PORT2: number;
  readonly PORT3: number;
  readonly PORT4: number;
  readonly PORT5: number;
  readonly PORT6: number;
  readonly uptime_ms: number;
  readonly signal: boolean;
};
/* tslint:enable:interface-over-type-literal */

@Injectable()
export class SimEmscriptenService {

  private _em_sim_instance;
  private _em_module;

  private get lazyEm() {
    if (!this._em_module) {
      // alert(42);
      this._em_module = EM_SIM_MODULE();
    }
    return this._em_module;
  }

  private get lazySim() {
    if (!this._em_sim_instance) {
      // alert(42);
      this._em_sim_instance = new this.lazyEm.MSP430();
    }
    return this._em_sim_instance;
  }

  FS_createDataFile(
      parent: string,
      name: string,
      data: Uint8Array,
      canRead: boolean = true): void {
    this.lazyEm.FS_createDataFile(parent, name, data, canRead);
  }

  init(fileName: string): void {
    const code = this.lazySim.init(fileName, 32768, 4e5, 6);
    if (code) {
      throw new Error(`EmSim: bad exit code ${code} !`);
    }
  }

  reset(): void {
    this.lazySim.reset();
  }

  bit_test(port: number, bit: number): boolean {
    /* tslint:disable:no-bitwise */
    return !!(port && ((1 << bit) & port));
    /* tslint:enable:no-bitwise */
  }

  step(p1_in = -1): {
      readonly UPTIME_MS: number,
      readonly sss: StepSnapshot_t[]} {

    const ss: StepSnapshot_t[] = this.lazySim.step(p1_in),
          ss_last = ss[ss.length - 1];

    if (ss_last.signal) {
      throw new Error(`EmSim: bad signal ${ss_last.signal} !`);
    }

    return {
      UPTIME_MS: ss_last.uptime_ms,
      sss: ss
    };
  }

  // reset runtime to clear ports e.t.c.
  end(): void {
    this.lazySim.end();
    this.lazySim.dump_stats();
    // TODO: possible memory leak
    this._em_sim_instance = this._em_module = null;
  }

}
