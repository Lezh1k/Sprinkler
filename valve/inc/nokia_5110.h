#ifndef NOKIA_5110_H
#define NOKIA_5110_H

#include <stdbool.h>
#include <stdint.h>

void nokia5110_init(void);
void nokia5110_clear(void);
void nokia5110_gotoXY(int8_t col, int8_t row);
void nokia5110_write_char(char c);
void nokia5110_write_str(const char *s);

#endif  // NOKIA_5110_H
