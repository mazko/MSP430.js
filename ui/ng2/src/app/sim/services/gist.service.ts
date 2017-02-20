import { Injectable } from '@angular/core';
import { Http, Response } from '@angular/http';
import 'rxjs/add/operator/toPromise';

// curl -I http://api.github.com/users/mazko  -> 301
// curl -I https://api.github.com/users/mazko -> 200
const GISTS_URL = 'https://api.github.com/gists';

interface IGistModel {
  elf: string;
  boardId: number;
}

@Injectable()
export class GistService {

  constructor(private _http: Http) { }

  fetch(id: string): Promise<IGistModel> {
    return this._http
      .get(GISTS_URL + '/' + id)
      .toPromise()
      .then(r => r.json().files)
      .then((files): IGistModel => ({
        elf: atob(files.program.content),
        boardId: files.boardId.content
      }));
  }

  create(model: IGistModel): Promise<{id: number, url: string}> {
    return this._http
      .post(GISTS_URL, {
        description: 'msp430.js',
        files: {
          program: {content: btoa(model.elf)},
          boardId: {content: `${model.boardId}`},
        }
      })
      .toPromise()
      .then(r => r.json())
      .then(({id, html_url}) => ({id, url: html_url}));
  }

}
