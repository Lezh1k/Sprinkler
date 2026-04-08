#include "commons.h"

#include <stdint.h>

void str_u8_2c(uint8_t val, char buff[3])
{
  uint8_t d0 = 0;
  buff[2] = '\0';
  while (val >= 10) {
    ++d0;
    val -= 10;
  }
  buff[1] = '0' + val;
  buff[0] = '0' + d0;
}
//////////////////////////////////////////////////////////////
