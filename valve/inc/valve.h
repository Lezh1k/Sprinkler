#ifndef VALVE_DFA_H
#define VALVE_DFA_H

#include <stdint.h>

#include "rt_clock.h"

typedef struct valve {
  int8_t pout;
  rtc_t open;
  rtc_t close;
} valve_t;
//////////////////////////////////////////////////////////////

void valves_init(valve_t **lst_valves, uint8_t *len);

rtc_t *valve_open_time(const valve_t *v);
rtc_t *valve_close_time(const valve_t *v);

#endif  // VALVE_DFA_H
