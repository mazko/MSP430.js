#include <stdint.h>


struct MSP430Snapshot {
  uint32_t signal, uptime_ms;
  uint8_t p1, p2, p3, p4, p5, p6;
};


class MSP430 {
public:

  int init(const std::string, const int, const int, const uint8_t);

  void reset();

  struct MSP430Snapshot step();

  void end();

private:
  struct elf32_struct_t * m_elf32;
};