#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

#define VALVES_DDR  DDRB
#define VALVES_PORT PORTB

static valve_t v0 = {
    .schedule =
        {
                   .strct =
                {
                    .open = {.time = {.hour = 0, .minute = 0, .second = 0}},
                    .close = {.time = {.hour = 2, .minute = 30, .second = 0}},
                }, },
    .port_n = PB0
};

static valve_t v1 = {
    .schedule =
        {
                   .strct =
                {
                    .open = {.time = {.hour = 2, .minute = 30, .second = 0}},
                    .close = {.time = {.hour = 5, .minute = 00, .second = 0}},
                }, },
    .port_n = PB1
};

static valve_t v2 = {
    .schedule =
        {
                   .strct =
                {
                    .open = {.time = {.hour = 5, .minute = 00, .second = 0}},
                    .close = {.time = {.hour = 7, .minute = 30, .second = 0}},
                }, },
    .port_n = PB2
};

static valve_t *valves[] = {&v0, &v1, &v2, NULL};
//////////////////////////////////////////////////////////////

void valves_init(valve_t ***lst_valves)
{
  VALVES_DDR |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
  for (valve_t **v = valves; *v; ++v)
    valve_close(*v);
  *lst_valves = valves;
}
//////////////////////////////////////////////////////////////

void valve_close(const valve_t *v)
{
  VALVES_PORT &= ~(1 << v->port_n);
}
//////////////////////////////////////////////////////////////

void valve_open(const valve_t *v)
{
  VALVES_PORT |= (1 << v->port_n);
}
//////////////////////////////////////////////////////////////
