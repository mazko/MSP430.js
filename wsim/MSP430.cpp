#include <stdint.h>

extern "C" {

  #include "./wsim-src/liblogger/logger.h"
  #include "./wsim-src/libelf/libelf.h"
  #include "./wsim-src/arch/common/mcu.h"

  #include "./wsim-src/machine/embind.h"
  wsimtime_t embind_nanotime; // MACHINE_TIME_GET_NANO()
  int embind_nanotime_incr; // MACHINE_TIME_GET_INCR()

  #include "./wsim-src/arch/common/debug.h"
  #define  EMMBIND_EXTERN_COMPILE_PATCH
  #include "./wsim-src/arch/msp430/msp430.h"
}


#include <iostream>
#include "MSP430.h"


int MSP430::init(const std::string s, const int xt1, const int xt2, const uint8_t verbose) {

  /* TODO: logger in constructor */

  logger_init("stdout", verbose  /* logger.h 0-6 */);

  int r = msp430_mcu_create(xt1, xt2);

  if (r) {
    // http://floooh.github.io/2016/08/27/asmjs-diet.html
    // std::cout << "EMBIND: msp430_mcu_create(): " << r << std::endl;
    return r;
  }

  this->m_elf32 = libelf_load(s.c_str(), verbose);

  if (this->m_elf32) {
    embind_nanotime = 0;
    embind_nanotime_incr = 0;
    MSP430::reset();
    return 0;
  } else {
    // http://floooh.github.io/2016/08/27/asmjs-diet.html
    // std::cout << "EMBIND: libelf_load() error" << std::endl;
    return 42;
  }
}


void MSP430::dump_stats() {
  mcu_dump_stats(0);
}


void MSP430::print_description() {
  mcu_print_description();
}


void MSP430::reset() {
  mcu_reset();
}


void MSP430::port_read(const uint8_t port_id, uint8_t& port) {
  msp430_digiIO_dev_read(port_id, &port);
  port &= msp430_digiIO_dev_read_dir(port_id);
}


void MSP430::run_and_fill_snapshot(struct MSP430Snapshot& snapshot) {
  mcu_run();         // MCU step and devices

  MSP430::port_read(PORT1, snapshot.p1);
  MSP430::port_read(PORT2, snapshot.p2);
  MSP430::port_read(PORT3, snapshot.p3);
  MSP430::port_read(PORT4, snapshot.p4);
  MSP430::port_read(PORT5, snapshot.p5);
  MSP430::port_read(PORT6, snapshot.p6);

  mcu_update_done(); // MCU step done
  embind_nanotime_incr = 0; // MACHINE_TIME_CLR_INCR();

  // snapshot.signal = mcu_signal_get();
  // snapshot.uptime_ms = embind_nanotime / 1000000UL;
}


#define EXPAND_TO_TEN(n) \
  MSP430::run_and_fill_snapshot(chunk.s##n##0); \
  MSP430::run_and_fill_snapshot(chunk.s##n##1); \
  MSP430::run_and_fill_snapshot(chunk.s##n##2); \
  MSP430::run_and_fill_snapshot(chunk.s##n##3); \
  MSP430::run_and_fill_snapshot(chunk.s##n##4); \
  MSP430::run_and_fill_snapshot(chunk.s##n##5); \
  MSP430::run_and_fill_snapshot(chunk.s##n##6); \
  MSP430::run_and_fill_snapshot(chunk.s##n##7); \
  MSP430::run_and_fill_snapshot(chunk.s##n##8); \
  MSP430::run_and_fill_snapshot(chunk.s##n##9)

struct MSP430Chunk MSP430::step() {

  struct MSP430Chunk chunk = {{0}};

  EXPAND_TO_TEN();
  EXPAND_TO_TEN(1);
  EXPAND_TO_TEN(2);
  EXPAND_TO_TEN(3);
  EXPAND_TO_TEN(4);
  EXPAND_TO_TEN(5);
  EXPAND_TO_TEN(6);
  EXPAND_TO_TEN(7);
  EXPAND_TO_TEN(8);
  EXPAND_TO_TEN(9);

  chunk.s99.signal = mcu_signal_get();
  chunk.s99.uptime_ms = embind_nanotime / 1000000UL;

  return chunk;
}

#undef EXPAND_TO_TEN


void MSP430::end() {
  libelf_close(this->m_elf32);
}
