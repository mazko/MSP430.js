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


const routes: Routes = [
  { path: 'board/1', component: Board1Component },
  { path: 'board/2', component: Board2Component },
  { path: '', redirectTo: 'board/1', pathMatch: 'full' },
  { path: ':id', component: GistComponent },
];


@NgModule({
  declarations: [
    AppComponent,
    SimComponent,
    Board1Component,
    Board2Component,
    GistComponent
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