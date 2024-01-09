#define F_CPU (16000000 / 8)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdint.h>

#include "nokia_5110.h"
#include "valve.h"

#define LED_GRN_DDR   DDRD
#define LED_GRN_DDR_N DDD5
#define LED_GRN_PORT  PORTD
#define LED_GRN_PORTN PD5

static volatile uint8_t SOFT_INTERRUPTS_REG = 0;
enum soft_interrupts_flags {
  SIVF_1SEC_PASSED = 1 << 1,
};
//////////////////////////////////////////////////////////////

static inline void led_grn_init(void)
{
  LED_GRN_DDR |= (1 << LED_GRN_DDR_N);
}
static void led_grn_turn_on(bool on);
//////////////////////////////////////////////////////////////

static volatile bool ocr1a_compensation = true;
static const uint16_t ocr1a_vals[2] = {7812, 7813};

ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0;
  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  ocr1a_compensation = !ocr1a_compensation;
  led_grn_turn_on(!ocr1a_compensation);  // just for indication that timer works
  SOFT_INTERRUPTS_REG |= SIVF_1SEC_PASSED;
}
//////////////////////////////////////////////////////////////

static void timer1_compa_int_enable(void)
{
  TIMSK |= (1 << OCIE1A);
}
//////////////////////////////////////////////////////////////

static void timer1_init(void)
{
  TCCR1B = (1 << CS12);  // 256 prescaler
  timer1_compa_int_enable();
  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  TCNT1 = 0;
}
//////////////////////////////////////////////////////////////

int main(void)
{
  bool lge = false;
  nokia5110_init();
  nokia5110_gotoXY(0, 0);
  nokia5110_write_str_s("hello", 3);
  
  led_grn_init();
  valve_init();
  timer1_init();
  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  while (true) {
    sleep_cpu();
    if (SOFT_INTERRUPTS_REG & SIVF_1SEC_PASSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_1SEC_PASSED;
      valve_sec_passed();
      led_grn_turn_on(lge = !lge);
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////

void led_grn_turn_on(bool on)
{
  if (on)
    LED_GRN_PORT |= (1 << LED_GRN_PORTN);
  else
    LED_GRN_PORT &= ~(1 << LED_GRN_PORTN);
}
//////////////////////////////////////////////////////////////
