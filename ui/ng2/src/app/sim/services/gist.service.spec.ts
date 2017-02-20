/* tslint:disable:no-unused-variable */

import { TestBed, async, inject } from '@angular/core/testing';
import { GistService } from './gist.service';

describe('GistService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [GistService]
    });
  });

  it('should ...', inject([GistService], (service: GistService) => {
    expect(service).toBeTruthy();
  }));
});
