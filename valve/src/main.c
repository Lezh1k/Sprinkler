#define F_CPU (16000000 / 8)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdbool.h>
#include "valve.h"

#define LED_GRN_DDR DDD5
#define LED_GRN_PORT PORTD
#define LED_GRN_PORTN PD5

static volatile uint8_t SOFT_INTERRUPTS_REG = 0;
enum soft_interrupts_flags {
  SIVF_1SEC_PASSED = 1 << 1,
};

static void led_grn_turn_on(bool on);
static inline void timer1_compa_int_enable() { TIMSK |= (1 << OCIE1A); }
//////////////////////////////////////////////////////////////

// because 1sec = 7812.5 cicles on this F_CPU
// F_CPU = 16 000 000 / 2
// F_TIMER1_PRESC_256 = F_CPU / 256
static volatile bool ocr1a_compensation = true;
static const uint16_t ocr1a_vals[2] = {7812, 7813};

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  ocr1a_compensation = !ocr1a_compensation;
  led_grn_turn_on(!ocr1a_compensation); //just for indication that timer works
  SOFT_INTERRUPTS_REG |= SIVF_1SEC_PASSED;
}
//////////////////////////////////////////////////////////////

int main(void) {
  DDRD |= (1 << LED_GRN_DDR);
  valve_init();
  TCCR1B = (1 << CS12); // 256 prescaler
  timer1_compa_int_enable();

  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  TCNT1 = 0;
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);
  while (true) {
    sleep_enable();
    sleep_cpu();

    if (SOFT_INTERRUPTS_REG & SIVF_1SEC_PASSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_1SEC_PASSED;
      valve_sec_passed();
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////

void led_grn_turn_on(bool on) {
  if (on)
    LED_GRN_PORT |= (1 << LED_GRN_PORTN);
  else
    LED_GRN_PORT &= ~(1 << LED_GRN_PORTN);
}
//////////////////////////////////////////////////////////////
