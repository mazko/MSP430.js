import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Params, Router } from '@angular/router';
import { GistService } from '../services/gist.service';
import { ControllerService } from '../services/controller.service';
import { StateService } from '../services/state.service';

import 'rxjs/add/operator/switchMap';


// http://localhost:4200/0791e557edbda6359ac1ffa87c4d1988


@Component({
  selector: 'app-gist',
  templateUrl: './gist.component.html',
  styleUrls: ['./gist.component.css']
})
export class GistComponent implements OnInit {

  constructor(
      private readonly _route: ActivatedRoute,
      private readonly _gist: GistService,
      private readonly _controller: ControllerService,
      private readonly _router: Router,
      private readonly _state: StateService) {}

  ngOnInit(): void {
    // alert(this._route.snapshot.params['id'])
    this._route.params
      .switchMap((params: Params) =>
        this._gist.fetch(params['id'])
      ).subscribe(
        ({elf, boardId}): void => {
          try {
            this._state.set_elf_file_from_string(elf);
            this._router.navigate(['/board/', boardId]);
          } catch (err) {
            this._controller.errorOccured(err);
          }
        },
        err => this._controller.errorOccured(err)
      );
  }

}
