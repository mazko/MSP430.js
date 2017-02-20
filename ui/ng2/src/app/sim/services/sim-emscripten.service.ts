import { Injectable } from '@angular/core';

// https://angular.io/docs/ts/latest/guide/dependency-injection.html#singleton-services

const EM_SIM_MODULE = window['msp430_sim_module'];


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
    const code = this.lazySim.init(fileName, 32768, 5e4, 6);
    if (code) {
      throw new Error(`EmSim: bad exit code ${code} !`);
    }
  }

  reset(): void {
    this.lazySim.reset();
  }

  step() {
    const snapshot = this.lazySim.step();
    if (snapshot.signal) {
      throw new Error(`EmSim: bad signal ${snapshot.signal} !`);
    }

    return new class {
      const PORT1: number = snapshot.PORT1;
      const PORT2: number = snapshot.PORT2;
      const PORT3: number = snapshot.PORT3;
      const PORT4: number = snapshot.PORT4;
      const PORT5: number = snapshot.PORT5;
      const PORT6: number = snapshot.PORT6;

      const UPTIME_MS = snapshot.uptime_ms;

      private bit_test(port: number, bit: number): boolean {
        /* tslint:disable:no-bitwise */
        return !!((1 << bit) & port);
        /* tslint:enable:no-bitwise */
      }

      get_port1_bit(bit: number): boolean {
        return this.bit_test(this.PORT1, bit);
      }

      get_port2_bit(bit: number): boolean {
        return this.bit_test(this.PORT2, bit);
      }

      get_port3_bit(bit: number): boolean {
        return this.bit_test(this.PORT3, bit);
      }

      get_port4_bit(bit: number): boolean {
        return this.bit_test(this.PORT4, bit);
      }

      get_port5_bit(bit: number): boolean {
        return this.bit_test(this.PORT5, bit);
      }

      get_port6_bit(bit: number): boolean {
        return this.bit_test(this.PORT6, bit);
      }

    };
  }

  // reset runtime to clear ports e.t.c.
  end(): void {
    this.lazySim.end();
    // TODO: possible memory leak
    this._em_sim_instance = this._em_module = null;
  }

}
