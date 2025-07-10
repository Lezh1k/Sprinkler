#include "commons.h"
#include <stdint.h>

char *str_u8_n(uint8_t val, char *buff, uint8_t n) {
  char *ptr = buff + n;
  *(--ptr) = '\0';

  while (val && ptr > buff) {
    *(--ptr) = '0' + (val % 10);
    val /= 10;
  }

  while (ptr > buff)
    *(--ptr) = '0';

  return ptr;
}
//////////////////////////////////////////////////////////////
