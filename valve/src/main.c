#define F_CPU (16000000 / 8)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdbool.h>

#define VALVE_DDR DDD5
#define LED_GRN_DDR DDD4

#define LED_GRN_PORT PORTD
#define VALVE_PORT PORTD

#define LED_GRN_PORTN PD5
#define VALVE_PORTN PD4

#define period_before_start_sec (__SECONDS_TO_00__)
#define period_valve_opened_sec (10)
#define period_valve_closed_sec (20)

typedef enum tim1_ocra_state {
  DTOS_WAIT_FOR_START = 0,
  DTOS_VALVE_OPEN,
  DTOS_VALVE_CLOSE,
  DTOS_LAST
} tim1_ocra_state_t ;

static volatile tim1_ocra_state_t tim1_cs = DTOS_WAIT_FOR_START;
static uint16_t periods_to_change_state[3] = {
  period_before_start_sec,
  period_valve_opened_sec,
  period_valve_closed_sec,
};
//////////////////////////////////////////////////////////////

// because 1sec = 7812.5 cicles on this F_CPU
// F_CPU = 16 000 000 / 2
// F_TIMER1_PRESC_256 = F_CPU / 256
static volatile bool ocr1a_compensation = true;
static const uint16_t ocr1a_vals[2] = {7812, 7813};

static void led_grn_turn_on(bool on);
static void valve_close(bool close);
static inline void timer1_compa_int_enable() { TIMSK |= (1 << OCIE1A); }
//////////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  ocr1a_compensation = !ocr1a_compensation;
  led_grn_turn_on(!ocr1a_compensation); //just for indication that timer works

  if (periods_to_change_state[tim1_cs]--)
    return;
  // change state!
  periods_to_change_state[DTOS_WAIT_FOR_START] = 0; // we do not need to wait again
  periods_to_change_state[DTOS_VALVE_OPEN] = period_valve_opened_sec;
  periods_to_change_state[DTOS_VALVE_CLOSE] = period_valve_closed_sec;
  tim1_cs = (tim1_cs + 1) % DTOS_LAST;
  bool valve_is_closed = tim1_cs != DTOS_VALVE_OPEN;
  valve_close(valve_is_closed);
}
//////////////////////////////////////////////////////////////

int main(void) {
  DDRD = (1 << VALVE_DDR) | (1 << LED_GRN_DDR);
  TCCR1B = (1 << CS12); // 256 prescaler
  timer1_compa_int_enable();
  valve_close(true);
  OCR1A = ocr1a_vals[(int)ocr1a_compensation];
  TCNT1 = 0;
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);
  while (true) {
    sleep_enable();
    sleep_cpu();
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

void valve_close(bool close) {
  // to close the valve need to set high level to relay
  if (close)
    VALVE_PORT |= (1 << VALVE_PORTN);
  else
    VALVE_PORT &= ~(1 << VALVE_PORTN);
}
//////////////////////////////////////////////////////////////
