#include "nokia_5110.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>

#include "nokia_5110_font.h"

// Controller port (meaning) : Display port
// PB7(USCK) : CLK
// PB6(MOSI) : DIN
// PB5(_) : DC (data/command mode)
// PB4(_) : CE (chip enable)
// PB3(_) : RST

#define DDR_SPI  DDRB
#define DDR_CLK  DDB7
#define DDR_MOSI DDB6
#define DDR_DC   DDB5
#define DDR_CE   DDB4
#define DDR_RST  DDB3

#define P_SPI  PORTB
#define P_CLK  PB7
#define P_MOSI PB6
#define P_DC   PB5
#define P_CE   PB4
#define P_RST  PB3

#define RST_HIGH()  (P_SPI |= (1 << P_RST))
#define RST_LOW()   (P_SPI &= ~(1 << P_RST))
#define CE_HIGH()   (P_SPI |= (1 << P_CE))
#define CE_LOW()    (P_SPI &= ~(1 << P_CE))
#define DC_HIGH()   (P_SPI |= (1 << P_DC))
#define DC_LOW()    (P_SPI &= ~(1 << P_DC))
#define MOSI_HIGH() (P_SPI |= (1 << P_MOSI))
#define MOSI_LOW()  (P_SPI &= ~(1 << P_MOSI))
#define CLK_HIGH()  (P_SPI |= (1 << P_CLK))
#define CLK_LOW()   (P_SPI &= ~(1 << P_CLK))

typedef enum DISPLAY_MODE {
  DM_COMMAND = 0,
  DM_DATA = 1,
} D_MODE;

#define DISPLAY_BANKS 6
#define DISPLAY_WIDTH 84

static void gpio_config(void);
static void write_byte(D_MODE mode, uint8_t b);

static inline void set_mode(D_MODE m)
{
  if (m == DM_COMMAND)
    DC_LOW();
  else
    DC_HIGH();
}
//////////////////////////////////////////////////////////////

void gpio_config(void)
{
  // everything to output
  DDR_SPI |= (1 << DDR_CLK) | (1 << DDR_MOSI) | (1 << DDR_DC) | (1 << DDR_CE) |
             (1 << DDR_RST);
}
//////////////////////////////////////////////////////////////

void write_byte(D_MODE mode, uint8_t b)
{
  CLK_LOW();
  CE_LOW();
  set_mode(mode);
  for (int8_t i = 0; i < 8; ++i) {
    if (b & 0x80) {
      MOSI_HIGH();
    } else {
      MOSI_LOW();
    }

    CLK_HIGH();
    b <<= 1;
    CLK_LOW();
  }

  CE_HIGH();
  CLK_HIGH();
}
//////////////////////////////////////////////////////////////

void nokia5110_init(void)
{
  /* Configure gpio pins */
  gpio_config();

  set_mode(DM_COMMAND);
  DC_HIGH();
  MOSI_HIGH();
  CLK_HIGH();
  CE_HIGH();

  /* Reset the LCD to a known state */
  RST_LOW();  // Set LCD reset = 0;
  for (uint8_t i = 0; i != 0xff; i++)
    ;  // wait for a while
  RST_HIGH();

  /* Configure LCD module */
  write_byte(DM_COMMAND, 0x21);  // Extended instruction set selected
  write_byte(DM_COMMAND, 0xc0);  // set default contrast (see set_contrast func)
                                 // (or set voltage 5V)
  write_byte(DM_COMMAND, 0x07);  // Set temperature control (TC2)
  write_byte(DM_COMMAND, 0x13);  // Set Bias for 1/48
  write_byte(DM_COMMAND, 0x20);  // Revert to standard instruction set
  write_byte(DM_COMMAND, 0x0c);  // Set display on in "normal" mode
  nokia5110_clear();
}
//////////////////////////////////////////////////////////////

void nokia5110_clear(void)
{
  nokia5110_gotoXY(0, 0);
  for (uint8_t y = 0; y < DISPLAY_BANKS; y++) {
    for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
      write_byte(DM_DATA, 0x00);
    }
  }
  nokia5110_gotoXY(0, 0);
}
//////////////////////////////////////////////////////////////

void nokia5110_gotoXY(int8_t col, int8_t row)
{
  col *= DISPLAY_BANKS;
  write_byte(DM_COMMAND, 0x40 | (uint8_t)row);
  write_byte(DM_COMMAND, 0x80 | (uint8_t)col);
}
//////////////////////////////////////////////////////////////

void nokia5110_write_char(char c)
{
  c -= 0x20;
  for (uint8_t line = 0; line < 4; ++line) {
    write_byte(DM_DATA, pgm_read_byte(&font4_8[(uint8_t)c][line]));
  }
  write_byte(DM_DATA, 0x00);
}
//////////////////////////////////////////////////////////////

void nokia5110_write_str(const char *s)
{
  while (*s) {
    nokia5110_write_char(*s++);
  }
}
//////////////////////////////////////////////////////////////
