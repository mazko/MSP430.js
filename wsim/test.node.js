'use strict'

const MSP430Module = require('./wsim-embind.js'),
      MSP430 = MSP430Module.MSP430,
      assert = require('assert'),
      fs = require('fs');


MSP430Module.FS_createDataFile('.','p.elf', fs.readFileSync('./blink/msp430f1611.out'), true);
const sim = new MSP430();
assert( sim.init('./p.elf', 32768, 1000000, 6 /* verbose 0-6 */) === 0 );
sim.print_description();


// test seq
(function() {
  const values = []; let offset = -1;
  for (let watchdog = 42; --watchdog;) {
    const snapshots = sim.step(-1);
    for (const snapshot of snapshots) {
      if (offset === -1 && snapshot.PORT5) {
        offset = values.length;
      }
      values.push(snapshot.PORT5);
    }
  }

  assert(offset > -1);
  assert(values.length === 100 * 41);
  assert(values.length >= offset + 242);
  assert(values[241 + offset] === 242, 
            values.slice(offset, offset + 241));

  for (let i = 0; i < 242; i++) {
    assert(values[i + offset] === i + 1, 
            values.slice(offset + i, offset + 241));
  }
})();


var realtime_last_sys = Date.now(),
    last_pin = null;

function next() {
  // console.log('p');

  let sim_delta, watchdog = 0;
  for (const date_delta = Date.now() - realtime_last_sys; watchdog < 42; watchdog++) {
    const snapshots = sim.step(-1);
    assert(snapshots.length === 100);
    const snapshot = snapshots[snapshots.length -1];
    assert(snapshot.signal === 0);
    assert(snapshot.uptime_ms >= 0);

    // after sim.step() before break
    var new_pin = snapshot.PORT5;
    if (new_pin !== last_pin){
      console.log(new_pin);
      last_pin = new_pin;
    }

    sim_delta = snapshot.uptime_ms - date_delta;
    if (sim_delta < -1000 /* 1 sec late, bad :( */) {
      console.log('WSIM: dt [' + sim_delta + '] is unadjustable, reset timer');
      /*

      delta = a - (b - c) = a - b + c = a - b + (c - (a - b + c)) = a - b + c - a + b - c = 0

      */
      realtime_last_sys -= sim_delta;
      break;
    }
    if (sim_delta > 100 /* good */) {
      /* WSIM_REALTIME_PRECISION_SECONDS 0.05 */
      break;
    }
  }

  setTimeout(next, Math.max(0, sim_delta));
}
next();

process.on('SIGINT', function() {
    console.log("Caught interrupt signal...");
    sim.end();
    sim.dump_stats();
    process.exit();
});
