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
    let date_last = null, health_cache = null;

    const next = (): void => {

      if (date_last === null) {
        date_last = Date.now();
      }

      switch (this._simSatate) {

        case SimStateEnum.STOPPING:
          this._simSatate = SimStateEnum.READY;
          this._state.isRunning = false;
          this._sim.end();
          break;

        case SimStateEnum.RUNNING:
          let sim_delta;
          const ui_date_now = Date.now();
          do {
            const date_now = Date.now();
            sim_delta = this._do_step() - (date_now - date_last);
            // _do_step -> exception -> ui_catcher -> stop() -> SimStateEnum.STOPPING
            if (this._simSatate !== SimStateEnum.RUNNING) {
              next(); // cleanup
              return;
            }
            if (date_now - ui_date_now > 22 /* UI antifreeze */) {
              break;
            }
          } while (sim_delta < 42 /* 42ms ahead is enough :) */);

          // console.log(`ui_delta: ${Date.now() - ui_date_now}, sim_delta: ${sim_delta}`);

          if (sim_delta < -1000 /* 1 sec late, bad :( */) {
            console.log('WSIM: dt [' + sim_delta + '] is unadjustable, reset timer');
            date_last -= sim_delta;
          }

          this._do_apply_steps();

          if (health_cache !== (sim_delta > 0)) {
            health_cache = (sim_delta > 0);
            this._controller.healthChanged(health_cache);
          }

          setTimeout(next, Math.max(0, sim_delta));
          break;

        default:
          console.log('ERROR: this._simSatate == ' + this._simSatate);
      }
    };
    // start
    setTimeout(next, 0);
  }

  @ui_catcher
  private start(): void {
    if (this._simSatate === SimStateEnum.READY) {
      this._do_sim(); // deferred by setTimeout
      this._simSatate = SimStateEnum.RUNNING;
      this._state.isRunning = true;
      // exception -> ui_catcher -> stop() -> SimStateEnum.READY
      this._sim.init(this._state.elf);
    } else {
      throw new Error('Skip start. Not ready.');
    }
  }

}
