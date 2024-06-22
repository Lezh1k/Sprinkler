#ifndef VALVE_DFA_H
#define VALVE_DFA_H

#include "rt_clock.h"

typedef struct valve {
  rtc_t open;
  rtc_t close;
} valve_t;
//////////////////////////////////////////////////////////////

void valves_init(valve_t ***lst_valves);
void valve_close(uint8_t v_idx);
void valve_open(uint8_t v_idx);

#endif  // VALVE_DFA_H
