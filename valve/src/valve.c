#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>

#define VALVE_DDR   DDRB
#define VALVE_DDR_N DDB0
#define VALVE_PORT  PORTB
#define VALVE_PORTN PB0

#ifdef __VALVE_DEBUG__
#define period_before_start_sec ((uint32_t)5u)
#define period_valve_opened_sec ((uint32_t)5u)
#define period_valve_closed_sec ((uint32_t)10u)
#else
#define period_before_start_sec (__SECONDS_TO_00__)
#define period_valve_opened_sec ((uint32_t)(60u * 60u * 6u))
#define period_valve_closed_sec ((uint32_t)(60u * 60u * 18u))
#endif

typedef enum dfa_state_num {
  VDS_WAIT_FOR_START = 0,
  VDS_VALVE_OPENED,
  VDS_VALVE_CLOSED,
  // ...
  VDS_LAST
} dfa_state_num_t;

static void valve_close(bool close);
static dfa_state_num_t m_cs = VDS_WAIT_FOR_START;
static uint32_t m_periods[3] = {
    period_before_start_sec,
    period_valve_opened_sec,
    period_valve_closed_sec,
};
//////////////////////////////////////////////////////////////

void valve_init(void)
{
  VALVE_DDR |= (1 << VALVE_DDR_N);
  valve_close(true);
}
//////////////////////////////////////////////////////////////

void valve_sec_passed(void)
{
  if (m_periods[m_cs]--)
    return;  // do nothing
  // change state!
  m_periods[VDS_WAIT_FOR_START] = 0;  // we do not need to wait again
  m_periods[VDS_VALVE_OPENED] = period_valve_opened_sec;
  m_periods[VDS_VALVE_CLOSED] = period_valve_closed_sec;
  m_cs = (m_cs + 1) % VDS_LAST;
  bool valve_is_closed = m_cs != VDS_VALVE_OPENED;
  valve_close(valve_is_closed);
}
//////////////////////////////////////////////////////////////

void valve_close(bool close)
{
  // to close the valve need to set high level to relay
  if (close)
    VALVE_PORT |= (1 << VALVE_PORTN);
  else
    VALVE_PORT &= ~(1 << VALVE_PORTN);
}
//////////////////////////////////////////////////////////////
