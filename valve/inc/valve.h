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

void valve_set_open_time(valve_t *v, rtc_t rtc);
const rtc_t *valve_get_open_time(const valve_t *v);
void valve_set_close_time(valve_t *v, rtc_t rtc);
const rtc_t *valve_get_close_time(const valve_t *v);

#endif  // VALVE_DFA_H
