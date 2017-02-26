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
      .function("dump_stats", &MSP430::dump_stats)
      .function("print_description", &MSP430::print_description)
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


    #define EXPAND_TO_TEN(n) \
      .element(&MSP430Chunk::s##n##0) \
      .element(&MSP430Chunk::s##n##1) \
      .element(&MSP430Chunk::s##n##2) \
      .element(&MSP430Chunk::s##n##3) \
      .element(&MSP430Chunk::s##n##4) \
      .element(&MSP430Chunk::s##n##5) \
      .element(&MSP430Chunk::s##n##6) \
      .element(&MSP430Chunk::s##n##7) \
      .element(&MSP430Chunk::s##n##8) \
      .element(&MSP430Chunk::s##n##9)

    value_array<MSP430Chunk>("MSP430Chunk")
      EXPAND_TO_TEN()
      EXPAND_TO_TEN(1)
      EXPAND_TO_TEN(2)
      EXPAND_TO_TEN(3)
      EXPAND_TO_TEN(4)
      EXPAND_TO_TEN(5)
      EXPAND_TO_TEN(6)
      EXPAND_TO_TEN(7)
      EXPAND_TO_TEN(8)
      EXPAND_TO_TEN(9)
      ;

    #undef EXPAND_TO_TEN

}