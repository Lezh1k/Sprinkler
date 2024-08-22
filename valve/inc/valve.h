#ifndef VALVE_DFA_H
#define VALVE_DFA_H

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

void valves_init(valve_t ***lst_valves);
void valve_close(const valve_t *v);
void valve_open(const valve_t *v);

#endif  // VALVE_DFA_H
