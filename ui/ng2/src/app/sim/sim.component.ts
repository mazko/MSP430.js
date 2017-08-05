import { Component } from '@angular/core';
import { GistService } from './services/gist.service';
import { ControllerService } from './services/controller.service';
import { StateService } from './services/state.service';


type BsAlertType = 'success' | 'info' | 'danger';


@Component({
  selector: 'app-sim',
  templateUrl: './sim.component.html',
  styleUrls: ['./sim.component.css'],
  providers: [ControllerService, StateService],
})
export class SimComponent {

  isFreezed = false;

  readonly boards: {name: string, id: number}[] = [
    { name: 'MSP430F1611 & 16xRGB Circle',  id: 1 },
    { name: 'MSP430F1611 & SSD1306 & 8 buttons & 10xRGB',  id: 2 },
      null, // divider
    { name: 'MSP430F1611 & RGB Circle | UART',  id: 4 },
    { name: 'MSP430F1611 & SSD1306 & 8 buttons & RGB | UART | ENC28J60',  id: 3 },
  ];

  readonly alerts: {msg: string, type: BsAlertType, html?: boolean}[] = [];

  gistBtnEnabled: boolean;

  // Angular2 template binding magic
  // detects changes even inside getters

  get isRunning(): boolean {
    return this._state.isRunning;
  }

  get hasElf(): boolean {
    return this._state.hasElf;
  }

  run_clicked(): void {
    if (this.isRunning) {
      this._controller.stopSimulation();
    } else {
      this._controller.startSimulation();
    }
  }

  gist_clicked(): void {
    this._gist
      .create({
        elf: this._state.elf_as_str,
        boardId: this._state.currentBoardId
      })
      .then(({id, url}): void => {
        // TODO: routerLink instead window.location
        this.alerts.push({
          msg: `<strong>GIST:</strong>
            <a href='${url}'>${id}</a> | <strong>Share:</strong>
            <a href='${window.location.protocol}//${window.location.host}/${id}'>me</a>`,
          type: 'success', html: true});
      })
      .catch(e => {
        console.log(e);
        this.alerts.push({msg: `Can't create gist, http error! ${e.statusText}`, type: 'danger'});
      });
  }

  elf_changed($event): void {
    for (const file of $event.target.files) {
      const reader = new FileReader();
      // https://github.com/Microsoft/TypeScript/issues/299#issuecomment-168538829
      reader.onload = (f: any): void => {
        const contents = new Uint8Array(f.target.result);
        this._state.set_elf_file(contents);
        this.gistBtnEnabled = true;
      };
      reader.readAsArrayBuffer(file);
    }
  }

  constructor(
      private readonly _gist: GistService,
      private readonly _controller: ControllerService,
      private readonly _state: StateService) {

    _controller
      .errorOccured$
      .subscribe((err): void => {
          console.log(`${err}`); // error short version
          this.alerts.push({msg: `${err}`, type: 'danger'});
      });

    _controller
      .healthChanged$
      .subscribe((heath): void => {
        // console.log(dt > interval)
        this.isFreezed = !heath;
      });
  }

}
