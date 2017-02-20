extern "C" {

  #include <stdint.h>
  #include "./wsim-src/libtracer/tracer.h"


  static tracer_id_t   tracer_registered_id = 0; /* registered id from simulation */


  void  tracer_start              (void) {}

  void  tracer_stop               (void) {}

  tracer_id_t tracer_event_add_id (int width, const char* label, const char* module) {
    return ++tracer_registered_id;
  }

  // void tracer_event_record        (tracer_id_t id, tracer_val_t val) {}

}