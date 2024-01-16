#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdint.h>

/// str_i8 converts int8_t value into string
/// val - value
/// buff - destination buffer (at least (and most actually) 5 bytes)
char *str_i8(int8_t val, char *buff);
char *str_u8(uint8_t val, char *buff);

#endif
