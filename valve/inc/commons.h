#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdint.h>

#define FILL_CHAR_EMPTY -1

char *str_u8(uint8_t val, char *buff);
char *str_u8_n(uint8_t val, char *buff, uint8_t n, char fill_char);

#endif
