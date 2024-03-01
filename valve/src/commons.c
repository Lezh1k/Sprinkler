#include "commons.h"

#include <stdbool.h>

char *str_u8_n(uint8_t val, char *buff, uint8_t n, char fill_char)
{
  buff += --n;
  *buff-- = 0;

  if (val == 0) {
    *buff-- = '0';
    --n;
  }

  while (val && n--) {
    *buff-- = '0' + (val % 10);
    val /= 10;
  }

  if (fill_char == FILL_CHAR_EMPTY)
    return ++buff;

  while (n--) {
    *buff-- = fill_char;
  }

  return ++buff;
}
//////////////////////////////////////////////////////////////

char *str_u8(uint8_t val, char *buff)
{
  return str_u8_n(val, buff, 4, FILL_CHAR_EMPTY);
}
//////////////////////////////////////////////////////////////
