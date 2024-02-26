#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "rt_clock.h"

static void valve_close(const struct valve *v);
static void valve_open(const struct valve *v);

#define VALVES_DDR  DDRB
#define VALVES_PORT PORTB

void valves_init(valve_t **lst_valves, uint8_t *len)
{
  static valve_t valves[3] = {
      {.pout = PB0,
       .open = {.hour = 0, .minute = 0, .second = 0},
       .close = {.hour = 2, .minute = 30, .second = 0}},
      {.pout = PB1,
       .open = {.hour = 2, .minute = 30, .second = 0},
       .close = {.hour = 5, .minute = 0, .second = 0} },
      {.pout = PB2,
       .open = {.hour = 5, .minute = 0, .second = 0},
       .close = {.hour = 7, .minute = 30, .second = 0}},
  };

  for (uint8_t i = 0; i < 3; ++i) {
    VALVES_DDR |= (1 << i);
    valve_close(&valves[i]);
  }

  *lst_valves = valves;
  *len = 3;
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

rtc_t *valve_open_time(const valve_t *v)
{
  return &v->open;
}
//////////////////////////////////////////////////////////////

rtc_t *valve_close_time(const valve_t *v)
{
  return &v->close;
}
//////////////////////////////////////////////////////////////
