#include "valve.h"

#include <avr/eeprom.h>
#include <avr/io.h>

#define VALVES_DDR  DDRB
#define VALVES_PORT PORTB
#define VALVES_N    3

// clang-format off
static valve_t valves[VALVES_N] = {
  {
    .schedule =
    {
      .strct =
      {
        .open = {.time = {.hour = 0, .minute = 0, .second = 0}},
        .close = {.time = {.hour = 2, .minute = 30, .second = 0}},
      },
    },
    .port_n = PB0,
  },
  {
    .schedule =
    {
      .strct =
      {
        .open = {.time = {.hour = 2, .minute = 30, .second = 0}},
        .close = {.time = {.hour = 5, .minute = 0, .second = 0}},
      },
    },
    .port_n = PB1,
  },
  {
    .schedule =
    {
      .strct =
      {
        .open = {.time = {.hour = 5, .minute = 0, .second = 0}},
        .close = {.time = {.hour = 7, .minute = 30, .second = 0}},
      },
    },
    .port_n = PB2,
  },
};
// clang-format on
//////////////////////////////////////////////////////////////

typedef rtc_t eeprom_schedule_t[VALVES_N][2];
static eeprom_schedule_t EEMEM persisted_schedule = {
    {
     {.time = {.hour = 0, .minute = 0, .second = 0}},
     {.time = {.hour = 2, .minute = 30, .second = 0}},
     },
    {
     {.time = {.hour = 2, .minute = 30, .second = 0}},
     {.time = {.hour = 5, .minute = 0, .second = 0}},
     },
    {
     {.time = {.hour = 5, .minute = 0, .second = 0}},
     {.time = {.hour = 7, .minute = 30, .second = 0}},
     },
};
//////////////////////////////////////////////////////////////

void valves_init(valve_t **lst_valves, uint8_t *valves_n)
{
  VALVES_DDR |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
  eeprom_read_block(&valves[0].schedule.arr[0],
                    &persisted_schedule[0][0],
                    sizeof(persisted_schedule));
  // unnecessary valve_close because ports on power up are zeroed
  // for (uint8_t i = 0; i < VALVES_N; ++i)
  //   valve_close(&valves[i]);
  *lst_valves = valves;
  *valves_n = VALVES_N;
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

void valves_save_schedule(void)
{
  eeprom_update_block(&valves[0].schedule.arr[0],
                      &persisted_schedule[0][0],
                      sizeof(persisted_schedule));
}
//////////////////////////////////////////////////////////////
