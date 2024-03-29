#ifndef VALVE_DFA_H
#define VALVE_DFA_H

#include <stdint.h>

#include "rt_clock.h"

typedef struct valve {
  const int8_t pout;
  rtc_t open;
  rtc_t close;
} valve_t;
//////////////////////////////////////////////////////////////

void valves_init(valve_t ***lst_valves);
void valve_close(const struct valve *v);
void valve_open(const struct valve *v);

#endif  // VALVE_DFA_H
