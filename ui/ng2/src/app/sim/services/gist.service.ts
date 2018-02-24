import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders } from '@angular/common/http';
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

  constructor(private _http: HttpClient) { }

  fetch(id: string): Promise<IGistModel> {
    return this._http
      .get<any>(GISTS_URL + '/' + id)
      .toPromise()
      .then(r => r.files)
      .then((files): IGistModel => ({
        elf: atob(files.program.content),
        boardId: files.boardId.content
      }));
  }

  create(model: IGistModel): Promise<{id: number, url: string}> {
    const httpOptions = {
      headers: new HttpHeaders({
        'Authorization': 'Basic bWF6a29ib3Q6YmE5Yjc3YzQyYjZhNDhhMjM1YTE5NDk5NWFiNWQ4NzAyNmY0NmJjYw==',
      })
    };

    return this._http
      .post<any>(GISTS_URL, {
        description: 'msp430.js',
        files: {
          program: {content: btoa(model.elf)},
          boardId: {content: `${model.boardId}`},
        }
      }, httpOptions)
      .toPromise()
      .then(({id, html_url}) => ({id, url: html_url}));
  }

}
