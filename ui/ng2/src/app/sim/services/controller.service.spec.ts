/* tslint:disable:no-unused-variable */

import { TestBed, async, inject } from '@angular/core/testing';
import { ControllerService } from './controller.service';

describe('ControllerService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ControllerService]
    });
  });

  it('should ...', inject([ControllerService], (service: ControllerService) => {
    expect(service).toBeTruthy();
  }));
});
