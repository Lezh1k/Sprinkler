#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

#define VALVES_DDR  DDRB
#define VALVES_PORT PORTB

static valve_t v0 = {
    .open = {.time = {.hour = 0, .minute = 0, .second = 0}},
    .close = {.time = {.hour = 2, .minute = 30, .second = 0}},
};
static valve_t v1 = {
    .open = {.time = {.hour = 2, .minute = 30, .second = 0}},
    .close = {.time = {.hour = 5, .minute = 00, .second = 0}},
};
static valve_t v2 = {
    .open = {.time = {.hour = 5, .minute = 00, .second = 0}},
    .close = {.time = {.hour = 7, .minute = 30, .second = 0}},
};

static const int8_t ports[] = {PB0, PB1, PB2};
static valve_t *valves[] = {&v0, &v1, &v2, NULL};
//////////////////////////////////////////////////////////////

void valves_init(valve_t ***lst_valves)
{
  VALVES_DDR |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
  for (int8_t i = 0; i < 3; ++i)
    valve_close(i);
  *lst_valves = valves;
}
//////////////////////////////////////////////////////////////

void valve_close(uint8_t v_idx)
{
  VALVES_PORT &= ~(1 << ports[v_idx]);
}
//////////////////////////////////////////////////////////////

void valve_open(uint8_t v_idx)
{
  VALVES_PORT |= (1 << ports[v_idx]);
}
//////////////////////////////////////////////////////////////
