import { OnDestroy, Injectable } from '@angular/core';
import { SimEmscriptenService } from '../services/sim-emscripten.service';
import { ControllerService } from '../services/controller.service';
import { StateService } from '../services/state.service';
import { Subscription } from 'rxjs/Subscription';
import { ui_catcher } from './decorators';


enum SimStateEnum {
  READY,
  RUNNING,
  STOPPING
}

/*
  That decorator makes it possible for Angular
  to identify the types of its dependencies - ControllerService etc.
  The SimComponent class had two dependencies
  as well but no @Injectable(). It didn't need
  @Injectable() because that component class has
  the @Component decorator. In Angular with TypeScript,
  a single decorator — any decorator — is sufficient
  to identify dependency types.
*/

@Injectable()
export abstract class AbstractBoard implements OnDestroy {

  private _simSatate = SimStateEnum.READY;

  protected readonly _subscriptions: Subscription[] = [];

  constructor(
      protected readonly _sim: SimEmscriptenService,
      protected readonly _state: StateService,
      readonly _controller: ControllerService) {

    this._subscriptions.push(_controller
      .simulationStarted$
      .subscribe(this.start.bind(this))
    );

    this._subscriptions.push(_controller
      .simulationStopped$
      .subscribe(this.stop.bind(this))
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
    if (this._simSatate === SimStateEnum.RUNNING) {
      this._simSatate = SimStateEnum.STOPPING;
    }
  }

  protected abstract _do_step(): number;
  protected abstract _do_apply_steps(): void;

  private _do_sim(): void {
    let date_last = Date.now(), health_cache = null;

    const next = (): void => {

      if (this._simSatate === SimStateEnum.STOPPING) {
        this._simSatate = SimStateEnum.READY;
        this._state.isRunning = false;
        this._sim.end();
      } else {

        const date_delta = Date.now() - date_last;
        let sim_delta = this._do_step() - date_delta, watchdog = 1;

        if (sim_delta < -1000 /* 1 sec late, bad :( */) {
          console.log('WSIM: dt [' + sim_delta + '] is unadjustable, reset timer');
          date_last -= sim_delta;
        } else {
          while (++watchdog < 42 && this._simSatate === SimStateEnum.RUNNING) {
            sim_delta = this._do_step() - date_delta;
            if (sim_delta > 100 /* 100ms ahead, good :) */) {
              break;
            }
          }
        }

        this._do_apply_steps();

        if (health_cache !== (sim_delta > 0)) {
          health_cache = (sim_delta > 0);
          this._controller.healthChanged(health_cache);
        }

        setTimeout(next, Math.max(0, sim_delta));
      }
    };
    // start
    setTimeout(next, 0);
  }

  @ui_catcher
  private start(): void {
    if (this._simSatate === SimStateEnum.READY) {
      try {
        this._state.write_elf_file_to_fs();
        this._sim.init(this._state.ELF_FILE_NAME);
      } catch (e) {
        this._sim.end();
        throw e;
      }
      this._do_sim();
      this._simSatate = SimStateEnum.RUNNING;
      this._state.isRunning = true;
    } else {
      throw new Error('Skip start. Not ready.');
    }
  }

}
