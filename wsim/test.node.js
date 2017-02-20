'use strict'

const MSP430Module = require('./wsim-embind.js'),
      MSP430 = MSP430Module.MSP430,
      assert = require('assert'),
      fs = require('fs');


MSP430Module.FS_createDataFile('.','p.elf', fs.readFileSync('./blink/msp430f1611.out'), true);
const sim = new MSP430();
assert( sim.init('./p.elf', 32768, 1000000, 6 /* verbose 0-6 */) === 0 );


var realtime_last_sys = Date.now(),
    last_pin = null;

function precT() {
  // console.log('p');
  var prec_delta = null;
  
  for (var watcdog = 0; watcdog < 1000; watcdog++) {
    const snapshot = sim.step();
    assert(snapshot.signal === 0);
    assert(snapshot.uptime_ms >= 0);
    prec_delta = snapshot.uptime_ms - (Date.now() - realtime_last_sys);

    // after sim.step() before break
    var new_pin = snapshot.PORT5;
    if (new_pin !== last_pin){
      console.log(new_pin);
      last_pin = new_pin;
    }

    if (prec_delta > 50 /* WSIM_REALTIME_PRECISION_SECONDS 0.05 */) break;
  }

  assert(prec_delta !== null);

  if (prec_delta > 0) {
    setTimeout(precT, prec_delta);
    //console.log('good: ' + delta);
  } else {
    // console.log('wsim is late: ' + prec_delta);
    setTimeout(precT, 0);
  }

  if (prec_delta < -1000) {
    console.log('WSIM: dt [' + prec_delta + '] is unadjustable, reset timer');

    /*

    delta = a - (b - c) = a - b + c = a - b + (c - (a - b + c)) = a - b + c - a + b - c = 0

    */

    realtime_last_sys -= prec_delta;
  }
}
precT();



// var last_pin = null;

// var realtime_last_wsim = 0, realtime_last_sys = Date.now();

// function step_delta() {
//   var current_wsim = sim.step();

//   assert(!realtime_last_wsim || current_wsim > 0);

//   var delta_wsim = current_wsim - realtime_last_wsim,
//       current_sys = Date.now(),
//       delta_sys = current_sys - realtime_last_sys,
//       delta = delta_wsim - delta_sys;

//   realtime_last_wsim = current_wsim;
//   realtime_last_sys  = current_sys;

//   assert(delta_wsim >= 0 && delta_sys >= 0);
  
//   var new_pin = sim.get_port5();
//   if (new_pin !== last_pin){
//     console.log(new_pin);
//     last_pin = new_pin;
//   }

//   return delta;
// }

// var prec_delta = 0;
// function precT() {
//   // console.log('p');
  
//   for (var watcdog = 0; watcdog < 10000; watcdog++) {
//     prec_delta += step_delta();
//     if (prec_delta > 5 /* WSIM_REALTIME_PRECISION_SECONDS 0.05 */) break;
//   }

//   if (prec_delta >= 0) {
//     setTimeout(precT, prec_delta);
//     //console.log('good: ' + delta);
//   } else {
//     // console.log('wsim is late: ' + prec_delta);
//     setTimeout(precT, 0);
//   }

//   if (prec_delta < -1000) {
//     console.log('RESET: wsim is too late: [' + prec_delta + ']');
//     prec_delta = 0;
//   }
// }
// precT();

//sim.end();