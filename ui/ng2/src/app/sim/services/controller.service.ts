import { Injectable } from '@angular/core';
import { Subject } from 'rxjs/Subject';


// https://angular.io/docs/ts/latest/cookbook/component-communication.html#!#bidirectional-service


@Injectable()
export class ControllerService {

  // Observable sources parent --> child
  private readonly startSource = new Subject();
  private readonly stopSource = new Subject();

  // Observable sources child --> parent
  private readonly errorSource = new Subject<Error>();
  private readonly healthSource = new Subject<boolean>();

  // Observable streams parent --> child
  readonly simulationStarted$ = this.startSource.asObservable();
  readonly simulationStopped$ = this.stopSource.asObservable();

  // Observable streams child --> parent
  readonly errorOccured$ = this.errorSource.asObservable();
  readonly healthChanged$ = this.healthSource.asObservable();

  // Service message commands parent --> child
  startSimulation(): void {
    this.startSource.next();
  }

  stopSimulation(): void {
    this.stopSource.next();
  }

  // Service message commands child --> parent

  errorOccured(error: Error): void {
    this.errorSource.next(error);
  }

  healthChanged(health: boolean): void {
    this.healthSource.next(health);
  }

  // https://basarat.gitbooks.io/typescript/docs/tips/bind.html
  readonly assert = (cond: boolean, err = new Error('Assert')): void => {
    if (!cond) {
      console.error(err);
      this.errorOccured(err);
      this.stopSimulation();
    }
  }

}
