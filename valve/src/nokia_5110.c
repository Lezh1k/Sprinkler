#include "nokia_5110.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

// Controller port (meaning) : Display port
// PB7(USCK) : CLK
// PB5(MOSI) : DIN
// PB4(_) : DC (data/command mode)
// PB3(_) : CE (chip enable)
// PB2(_) : RST  (TODO ADD TO CIRCUIT)

#define P_OUT PORTB
#define P_RST PB2
#define P_CE PB3
#define P_DC PB4
#define P_MOSI PB5
#define P_CLK PB7

#define RST_HIGH() (P_OUT |= (1 << P_RST))
#define RST_LOW() (P_OUT &= ~(1 << P_RST))
#define CE_HIGH() (P_OUT |= (1 << P_CE))
#define CE_LOW() (P_OUT &= ~(1 << P_CE))
#define DC_HIGH() (P_OUT |= (1 << P_DC))
#define DC_LOW() (P_OUT &= ~(1 << P_DC))
#define MOSI_HIGH() (P_OUT |= (1 << P_MOSI))
#define MOSI_LOW() (P_OUT &= ~(1 << P_MOSI))
#define CLK_HIGH() (P_OUT |= (1 << P_CLK))
#define CLK_LOW() (P_OUT &= ~(1 << P_CLK))

typedef enum {
  DM_COMMAND = 0,
  DM_DATA = 1,
} D_MODE; // DISPLAY MODE

static void gpio_config(void);
static void spi_config(void);
static void write_byte(D_MODE mode, uint8_t b);

static inline void set_mode(D_MODE m) {
  if (m == DM_COMMAND)
    DC_HIGH();
  else
    DC_LOW();
}
//////////////////////////////////////////////////////////////

static inline void chip_select(bool select) {
  if (select)
    CE_LOW();
  else
    CE_HIGH();
}

void gpio_config(void) {
  DDRB = (1 << DDB7) | (1 << DDB5) | (1 << DDB4) | (1 << DDB3);
}
//////////////////////////////////////////////////////////////

void spi_config(void) {}
//////////////////////////////////////////////////////////////

void nokia5110_init(void) {
  /* Configure gpio pins */
  gpio_config();
  /* Configure spi pins */
  spi_config();

  set_mode(DM_COMMAND);
  MOSI_HIGH();
  CLK_HIGH();
  chip_select(false);

  /* Reset the LCD to a known state */
  RST_LOW(); // Set LCD reset = 0;
  for (int16_t i = 0; i < 0xff; i++)
    ;         // WTF? we need to use something else.
  RST_HIGH(); // LCD_RST = 1

  /* Configure LCD module */
  write_byte(DM_COMMAND, 0x21); // Extended instruction set selected
  write_byte(DM_COMMAND,
             0xb7); // Set LCD voltage (defined by experimentation...)
  write_byte(DM_COMMAND, 0x04); // Set temperature control (TC2)
  write_byte(DM_COMMAND, 0x14); // Set Bias for 1/48
  write_byte(DM_COMMAND, 0x20); // Revert to standard instruction set
  write_byte(DM_COMMAND,
             0x0c); // Set display on in "normal" mode (not inversed)
  nokia5110_clear();
}
//////////////////////////////////////////////////////////////

void nokia5110_clear() {}
//////////////////////////////////////////////////////////////

void nokia5110_gotoXY(int8_t col, int8_t row) {}
//////////////////////////////////////////////////////////////

void nokia5110_write_char(char c) {}
//////////////////////////////////////////////////////////////

void nokia5110_write_str(const char *s) {}
//////////////////////////////////////////////////////////////

void nokia5110_write_str_s(const char *s, uint8_t len) {}
//////////////////////////////////////////////////////////////

void nokia5110_set_contrast(uint8_t contrast) {}
//////////////////////////////////////////////////////////////

void nokia5110_set_light(bool on) {}
//////////////////////////////////////////////////////////////

void nokia5110_set_pixel(int16_t x, int16_t y) {}
//////////////////////////////////////////////////////////////
