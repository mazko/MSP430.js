/*!
 * PicSim.js
 * http://msp430.js.org/
 * https://github.com/mazko/MSP430.js
 *
 * Copyright 2017, Oleg Mazko
 * http://www.opensource.org/licenses/bsd-license.html
 */


#include <emscripten/bind.h>
#include "MSP430.h"

// keep MSP430.cpp in own file to avoid 
// names conflicts with emscripten (#defines are EVIL) 

using namespace emscripten;

// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#a-quick-example

EMSCRIPTEN_BINDINGS(MSP430) {

    class_<MSP430>("MSP430")
      .constructor<>()
      .function("init", &MSP430::init)
      .function("reset", &MSP430::reset)
      .function("step", &MSP430::step)
      .function("end", &MSP430::end)
      ;


    value_object<MSP430Snapshot>("MSP430Snapshot")
      .field("signal", &MSP430Snapshot::signal)
      .field("uptime_ms", &MSP430Snapshot::uptime_ms)
      .field("PORT1", &MSP430Snapshot::p1)
      .field("PORT2", &MSP430Snapshot::p2)
      .field("PORT3", &MSP430Snapshot::p3)
      .field("PORT4", &MSP430Snapshot::p4)
      .field("PORT5", &MSP430Snapshot::p5)
      .field("PORT6", &MSP430Snapshot::p6)
      ;

}