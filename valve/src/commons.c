#include "commons.h"

#include <stdbool.h>

char *str_i8(int8_t val, char *buff)
{
  bool neg = val < 0;
  buff += 4;
  *buff-- = 0;
  while (val) {
    *buff-- = '0' + (val % 10);
    val /= 10;
  }
  if (neg)
    *buff-- = '-';
  return ++buff;
}
//////////////////////////////////////////////////////////////

char *str_u8(uint8_t val, char *buff)
{
  buff += 4;
  *buff-- = 0;
  while (val) {
    *buff-- = '0' + (val % 10);
    val /= 10;
  }
  return ++buff;
}
//////////////////////////////////////////////////////////////
