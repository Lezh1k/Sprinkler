#ifndef VALVE_DFA_H
#define VALVE_DFA_H

#include <stdint.h>

#include "rt_clock.h"

typedef struct valve {
  union {
    struct {
      rtc_t open;
      rtc_t close;
    } strct;
    rtc_t arr[2];
  } schedule;
  uint8_t port_n;
} valve_t;
//////////////////////////////////////////////////////////////

void valves_init(valve_t **lst_valves, uint8_t *valves_n);
void valve_close(const valve_t *v);
void valve_open(const valve_t *v);
void valves_save_schedule(void);

#endif  // VALVE_DFA_H
