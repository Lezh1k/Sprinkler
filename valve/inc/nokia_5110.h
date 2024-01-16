#ifndef NOKIA_5110_H
#define NOKIA_5110_H

#include <stdbool.h>
#include <stdint.h>

void nokia5110_init(void);
void nokia5110_clear(void);
void nokia5110_gotoXY(int8_t col, int8_t row);
void nokia5110_write_char(char c);
void nokia5110_write_str(const char *s);
void nokia5110_write_str_s(const char *s, uint8_t len);
void nokia5110_set_pixel(int16_t x, int16_t y);

#endif  // NOKIA_5110_H
