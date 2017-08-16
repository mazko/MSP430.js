import { Injectable } from '@angular/core';
import { ControllerService } from './controller.service';
import { StateService } from './state.service';


enum SimStateEnum {
  READY,
  CONNECTING,
  TIMEOUT,
  RUNNING,
  STOPPING
}


// enum state
@Injectable()
export class MspsimService {

  private _simState = SimStateEnum.READY;

  private _websocket: WebSocket;

  private readonly errorOccured: (error: Error) => void;

  private readonly healthChanged: (value: boolean) => void;

  private _any_button_pushed = false;

  constructor(controller: ControllerService,
              private readonly _state: StateService) {
    this.errorOccured = (e) => { controller.errorOccured(e); };
    this.healthChanged = (v) => { controller.healthChanged(v); };
  }

  private doSend(msg, unsafe: boolean = true): void {
    if (unsafe || this._websocket) {
      const what = JSON.stringify( msg );
      this._websocket.send( what );
    }
  }

  public button(id: number, value: boolean = true): void {
    if (value || this._any_button_pushed) {
      this._any_button_pushed = value;
      this.doSend({
        event: 'emulation_control',
        data: 'proiot$button' + id,
        proiot$value: value
      }, false);
    }
  }

  public stop(): void {
    if (this._simState === SimStateEnum.RUNNING) {
      this._simState = SimStateEnum.STOPPING;
      this._websocket.close();
    }
  }

  public start(jcls: string, onDataEvent: (name: string, data: object) => void): void {
    if (this._simState === SimStateEnum.READY) {
      this._state.isRunning = true;

      if (this._websocket) {
        throw new Error('Assert');
      }

      this._simState = SimStateEnum.CONNECTING;

      // ws://localhost:9148/emulink
      this._websocket = new WebSocket(
        `${window.location.protocol === 'https:' ? 'wss' : 'ws'}://${window.location.host}/emulink`);

      let timeoutID = null;

      this._websocket.onopen = () => {

          this.doSend({
            event: 'create',
            type: jcls,
            node: 'test_id'
          });

          this.doSend({
            event: 'write',
            node: 'test_id',
            elf8: Array.from(this._state.elf),
          });

          this.doSend({
            event: 'emulation_control',
            data: 'start',
          });

          const websocket = this._websocket;
          timeoutID = setTimeout(() => {
            if (websocket === this._websocket) {
              this._simState = SimStateEnum.TIMEOUT;
              websocket.close();
            }
          }, 12345);
      };

      let health_cache = null, health_timeoutId = null;

      const proceedJSON = (json) => {

        if (timeoutID !== null) {
          clearTimeout(timeoutID);
          timeoutID = null;
        }

        if (json.response === 'emulation_control' && json.data === 'start') {

          this._simState = SimStateEnum.RUNNING;

        } else if (json.proiot$event && this._simState === SimStateEnum.RUNNING) {

          onDataEvent(json.proiot$event, json.proiot$data);

          if (health_timeoutId !== null) {
            clearTimeout(health_timeoutId);
          }

          health_timeoutId = setTimeout(() => {
            if (health_cache === false && this._simState === SimStateEnum.RUNNING) {
              health_cache = true;
              this.healthChanged(health_cache);
            }
          }, 333);

          const health_current = (Date.now() - json.proiot$ms) < 333;
          if (health_cache !== health_current) {
            health_cache = health_current;
            this.healthChanged(health_cache);
          }

        }

      };


      this._websocket.onmessage = (evt) => {
          proceedJSON(JSON.parse(evt.data));
      };


      // https://stackoverflow.com/a/40084550
      this._websocket.onclose = (err) => {
        if (this._simState === SimStateEnum.TIMEOUT) {
          this.errorOccured(new Error('se.sics.mspsim.emulink.WSEmuLink is busy'));
        } else if (this._simState !== SimStateEnum.STOPPING) {
          console.log(`WS closed by error ? Code: ${err.code}, reason: ${err.reason}, wasClean: ${err.wasClean}`);
          this.errorOccured(new Error('java -cp mspsim.js.jar se.sics.mspsim.emulink.WSEmuLink'));
        }
        this._websocket = null;
        this._simState = SimStateEnum.READY;
        this._state.isRunning = false;
      };

    } else {
      throw new Error('Skip start. Not ready.');
    }

 }

}
