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

  mcu_print_description();

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

void MSP430::reset() {
  mcu_reset();
}

struct MSP430Snapshot MSP430::step() {

  struct MSP430Snapshot snapshot = {0};

  mcu_run();         // MCU step and devices

  msp430_digiIO_dev_read(PORT1, &snapshot.p1);
  msp430_digiIO_dev_read(PORT2, &snapshot.p2);
  msp430_digiIO_dev_read(PORT3, &snapshot.p3);
  msp430_digiIO_dev_read(PORT4, &snapshot.p4);
  msp430_digiIO_dev_read(PORT5, &snapshot.p5);
  msp430_digiIO_dev_read(PORT6, &snapshot.p6);

  mcu_update_done(); // MCU step done
  embind_nanotime_incr = 0; // MACHINE_TIME_CLR_INCR();

  snapshot.signal = mcu_signal_get();
  snapshot.uptime_ms = embind_nanotime / 1000000;

  return snapshot;
}

void MSP430::end() {
  libelf_close(this->m_elf32);
}
