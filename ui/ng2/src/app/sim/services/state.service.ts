import { Injectable } from '@angular/core';
import { Subject } from 'rxjs/Subject';
import { Subscription } from 'rxjs/Subscription';


type StateType =
  'simulation' |
  'elf'        |
  'board'      |
  'chip'
  ;


// http://stackoverflow.com/q/30689324/
// http://stackoverflow.com/q/29775830/

function state_prop(newState: StateType): Function {
  return function (target: Object, name: string): void {
    const _name = '___sim_private_only_symbol_' + name;
    // const _name = Symbol(name);
    Object.defineProperty(target, name, {
      get: function() {
        return this[_name];
      },
      set: function(value) {
        if (this[_name] !== value) {
          this[_name] = value;
          // TODO: this type check as StateService
          // some friendly class / Java-like default ?
          const this_: StateService = this;
          this_.___sim_private_only_method_notifyState(newState);
        }
      },
      enumerable: true,
      configurable: true
    });
  };
}


@Injectable()
export class StateService {

  // State Event

  private readonly stateSource = new Subject<StateType>();
  private readonly stateChanged$ = this.stateSource.asObservable();

  // Angular2 template binding magic
  // detects changes even inside getters

  @state_prop('board') currentBoardId: number;

  @state_prop('simulation') isRunning = false;

  @state_prop('chip') chip: number;

  get hasElf(): boolean {
    return !!this._elf_contents;
  }

  get elf_as_str(): string {
    return this._Uint8ToString(this._elf_contents);
  }

  get elf(): Uint8Array {
    // https://stackoverflow.com/a/22114687
    return new Uint8Array(this._elf_contents);
  }

  private _elf_contents: Uint8Array;

  subscribe(filterState: StateType, callback: () => void): Subscription {
    // http://stackoverflow.com/q/39494058/
    // remake of BehaviorSubject with event filter
    // and without mandatory default value
    callback();
    return this.stateChanged$.subscribe(
      (newState): void => {
        if (newState === filterState) {
          callback();
        }
      });
  }

  private _notifyState(newState: StateType): void {
    this.stateSource.next(newState);
  }

  ___sim_private_only_method_notifyState(newState: StateType): void {
    this._notifyState(newState);
  }

  private _Uint8ToString(u8a: Uint8Array): string {
    // http://stackoverflow.com/q/12710001/
    const CHUNK_SZ = 0x8000, c = [];
    for (let i = 0; i < u8a.length; i += CHUNK_SZ) {
      c.push(String.fromCharCode.apply(null, u8a.subarray(i, i + CHUNK_SZ)));
    }
    return c.join('');
  }

  set_elf_file(data: Uint8Array): void {
    this._elf_contents = data;
    this._notifyState('elf');
  }

  set_elf_file_from_string(data: string): void {
    this.set_elf_file(
      new Uint8Array(data.split('').map(c => c.charCodeAt(0))));
  }

}
