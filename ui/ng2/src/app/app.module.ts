import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpModule } from '@angular/http';
import { RouterModule, Routes } from '@angular/router';
import { Ng2BootstrapModule } from 'ngx-bootstrap';

import { SimEmscriptenService } from './sim/services/sim-emscripten.service';
import { GistService } from './sim/services/gist.service';


import { AppComponent } from './app.component';
import { SimComponent } from './sim/sim.component';
import { Board1Component } from './sim/boards/board1/board1.component';
import { Board2Component } from './sim/boards/board2/board2.component';
import { GistComponent } from './sim/gist/gist.component';
import { Board3Component } from './sim/boards/board3/board3.component';
import { Board4Component } from './sim/boards/board4/board4.component';


const routes: Routes = [
  { path: 'board/1', component: Board1Component },
  { path: 'board/2', component: Board2Component },
  { path: 'board/3', component: Board3Component },
  { path: 'board/4', component: Board4Component },
  { path: '', redirectTo: 'board/1', pathMatch: 'full' },
  { path: ':id', component: GistComponent },
];


@NgModule({
  declarations: [
    AppComponent,
    SimComponent,
    Board1Component,
    Board2Component,
    GistComponent,
    Board3Component,
    Board4Component
  ],
  imports: [
    Ng2BootstrapModule.forRoot(),
    BrowserModule,
    FormsModule,
    HttpModule,
    RouterModule.forRoot(routes)
  ],
  providers: [SimEmscriptenService, GistService],
  bootstrap: [AppComponent]
})
export class AppModule { }
