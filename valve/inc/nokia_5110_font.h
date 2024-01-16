#ifndef NOKIA_5110_FONT_H
#define NOKIA_5110_FONT_H

#include <avr/pgmspace.h>
extern const uint8_t font6_8[94][6] PROGMEM;
/* extern const uint8_t font12_16[94][6] PROGMEM; */

typedef struct nokia5110_font {
  uint8_t width;
  uint8_t height;
} nokia5110_font_t;

nokia5110_font_t font_6x8(void);

#endif
