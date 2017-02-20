/* tslint:disable:no-unused-variable */

import { TestBed, async, inject } from '@angular/core/testing';
import { SimEmscriptenService } from './sim-emscripten.service';

describe('SimEmscriptenService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [SimEmscriptenService]
    });
  });

  it('should ...', inject([SimEmscriptenService], (service: SimEmscriptenService) => {
    expect(service).toBeTruthy();
  }));
});
