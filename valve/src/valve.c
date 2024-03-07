#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VALVES_DDR  DDRB
#define VALVES_PORT PORTB

void valves_init(valve_t ***lst_valves)
{
  static valve_t v0 = {
      .pout = PB0,
      .open = {.time = {.hour = 0, .minute = 0, .second = 0}},
      .close = {.time = {.hour = 2, .minute = 30, .second = 0}},
  };
  static valve_t v1 = {
      .pout = PB1,
      .open = {.time = {.hour = 2, .minute = 30, .second = 0}},
      .close = {.time = {.hour = 5, .minute = 00, .second = 0}},
  };
  static valve_t v2 = {
      .pout = PB2,
      .open = {.time = {.hour = 5, .minute = 00, .second = 0}},
      .close = {.time = {.hour = 7, .minute = 30, .second = 0}},
  };
  static valve_t *valves[] = {&v0, &v1, &v2, NULL};

  for (uint8_t i = 0; i < 3; ++i) {
    VALVES_DDR |= (1 << i);
    valve_close(valves[i]);
  }

  *lst_valves = valves;
}
//////////////////////////////////////////////////////////////

void valve_close(const struct valve *v)
{
  VALVES_PORT |= (1 << v->pout);
}
//////////////////////////////////////////////////////////////

void valve_open(const struct valve *v)
{
  VALVES_PORT &= ~(1 << v->pout);
}
//////////////////////////////////////////////////////////////
