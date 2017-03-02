#include <stdint.h>


struct MSP430Snapshot {
  uint32_t signal, uptime_ms;
  uint8_t p1, p2, p3, p4, p5, p6;
};


#define EXPAND_TO_TEN(n) \
  s##n##0, s##n##1, s##n##2, s##n##3, s##n##4, s##n##5, s##n##6, s##n##7, s##n##8, s##n##9

struct MSP430Chunk {
  struct MSP430Snapshot EXPAND_TO_TEN(),
                        EXPAND_TO_TEN(1), 
                        EXPAND_TO_TEN(2),
                        EXPAND_TO_TEN(3),
                        EXPAND_TO_TEN(4),
                        EXPAND_TO_TEN(5),
                        EXPAND_TO_TEN(6),
                        EXPAND_TO_TEN(7),
                        EXPAND_TO_TEN(8),
                        EXPAND_TO_TEN(9);
};

#undef EXPAND_TO_TEN


class MSP430 {
public:

  int init(const std::string, const int, const int, const uint8_t);

  void reset();

  struct MSP430Chunk step(const int16_t);

  void end();

  void print_description();

  void dump_stats();

private:
  struct elf32_struct_t * m_elf32;

  uint8_t m_p1_in;
  bool m_p1_in_b;

  void run_and_fill_snapshot(struct MSP430Snapshot&);

  void port_read(const uint8_t, uint8_t&);
  void port_write(const uint8_t, const uint8_t);
};