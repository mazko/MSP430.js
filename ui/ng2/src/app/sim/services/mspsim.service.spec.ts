import { TestBed, inject } from '@angular/core/testing';

import { MspsimService } from './mspsim.service';

describe('MspsimService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [MspsimService]
    });
  });

  it('should be created', inject([MspsimService], (service: MspsimService) => {
    expect(service).toBeTruthy();
  }));
});
