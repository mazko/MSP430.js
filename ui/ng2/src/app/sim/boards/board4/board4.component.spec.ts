import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { Board4Component } from './board4.component';

describe('Board4Component', () => {
  let component: Board4Component;
  let fixture: ComponentFixture<Board4Component>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ Board4Component ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(Board4Component);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
