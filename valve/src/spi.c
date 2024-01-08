#include "spi.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#define SPI_T0CA_VAL (0xff >> 2)

typedef struct send_dfa {
  const char *data;
  uint8_t data_len; // maybe uint16_t

  void (*cb_transmission_finished)(void);
} send_dfa_t;

static send_dfa_t m_send_dfa;
//////////////////////////////////////////////////////////////

#define timer_interrupt_enable() (TIMSK |= (1 << OCIE0A))
#define timer_interrupt_disable() (TIMSK &= ~(1 << OCIE0A))

ISR(TIMER0_COMPA_vect) {
  TCNT0 = 0;
  //  OCR0A = SPI_T0CA_VAL; // why not set it once in initialization?
}
//////////////////////////////////////////////////////////////

ISR(USI_OVERFLOW_vect) {
  timer_interrupt_disable();

  if (!++m_send_dfa.data) { // reached the end of str
    m_send_dfa.cb_transmission_finished();
    return;
  }

  TCNT0 = 0;
  timer_interrupt_enable();
}
//////////////////////////////////////////////////////////////

void spi_config(void) {
  // set up timer0
  TCCR0B = (1 << CS00); // no prescaler
  OCR0A = SPI_T0CA_VAL;

  // set up USI to three wire mode
  // (1 << USIOIE) - counter  overflow interrupt enable
  // (1 << USIWM0) - three wire mode
  USICR = (1 << USIOIE) | (1 << USIWM0);
}
//////////////////////////////////////////////////////////////

void spi_send_str_async(const char *data, uint8_t len,
                        void (*cb_transmission_finished)(void)) {
  m_send_dfa.data = data;
  m_send_dfa.data_len = len;
  m_send_dfa.cb_transmission_finished = cb_transmission_finished;
  timer_interrupt_enable(); // start DFA
}
//////////////////////////////////////////////////////////////
