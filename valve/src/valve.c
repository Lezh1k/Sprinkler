#include "valve.h"

#include <avr/io.h>
#include <stdbool.h>

#define VALVE_DDR   DDRB
#define VALVE_DDR_N DDB0
#define VALVE_PORT  PORTB
#define VALVE_PORTN PB0

void valve_init(void)
{
  VALVE_DDR |= (1 << VALVE_DDR_N);
  valve_close(true);
}
//////////////////////////////////////////////////////////////

void valve_close(bool close)
{
  if (close)
    VALVE_PORT |= (1 << VALVE_PORTN);
  else
    VALVE_PORT &= ~(1 << VALVE_PORTN);
}
//////////////////////////////////////////////////////////////
