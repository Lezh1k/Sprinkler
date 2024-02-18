// see fuses
#include "rt_clock.h"
#define F_CPU (16000000 / 8)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stdint.h>

#include "commons.h"
#include "nokia_5110.h"
#include "valve.h"

#define LED_GRN_DDR   DDRD
#define LED_GRN_DDR_N DDD6
#define LED_GRN_PORT  PORTD
#define LED_GRN_PORTN PD6

#define BTNS_DDR        DDRD
#define BTNS_PIN        PIND
#define BTNS_INT_DDR_N  DDD2
#define BTNS_INT_PIN    PIND2
#define BTN_ENTER_DDR_N DDD3
#define BTN_ENTER_PIN   PIND3
#define BTN_UP_DDR_N    DDD4
#define BTN_UP_PIN      PIND4
#define BTN_DOWN_DDR_N  DDD5
#define BTN_DOWN_PIN    PIND5

#define TICK_DURATION_MS 500
#define TICK_OCRA1_VAL   15625

static volatile uint8_t SOFT_INTERRUPTS_REG = 0;

enum soft_interrupts_flags {
  SIVF_TICK_PASSED = 1 << 1,
  SIVF_BTN_PRESSED = 1 << 2,
};
//////////////////////////////////////////////////////////////

// led green
static void led_grn_init(void);
static void led_grn_turn_on(bool on);

// buttons
static void btns_init(void);
static void btn_pressed(void);
static void btn_enter_pressed(void);
static void btn_up_pressed(void);
static void btn_down_pressed(void);

// display
static void rtc_print(const rtc_t *r);
//////////////////////////////////////////////////////////////

// menu
typedef struct menu {
  rtc_t current_time;
  valve_t *valves;
  uint8_t valves_len;
} menu_t;
static menu_t m_menu;

static void display_current_menu(void);
//////////////////////////////////////////////////////////////

// timer 1
ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0;
  OCR1A = TICK_OCRA1_VAL;
  SOFT_INTERRUPTS_REG |= SIVF_TICK_PASSED;
}
//////////////////////////////////////////////////////////////

static void timer1_init(void)
{
  TCCR1B = (1 << CS11) | (1 << CS10);  // 64 prescaler
  TIMSK |= (1 << OCIE1A);              // timer1_compa_int_enable
  OCR1A = TICK_OCRA1_VAL;
  TCNT1 = 0;
}
//////////////////////////////////////////////////////////////

// btns
ISR(TIMER0_OVF_vect)
{
  // disable timer0_ovf interrupt
  TIMSK &= ~(1 << TOIE0);
  if ((BTNS_PIN & (1 << BTNS_INT_PIN)) == 0) {
    SOFT_INTERRUPTS_REG |= SIVF_BTN_PRESSED;
  }
  GIMSK = (1 << INT0);  // enable INT0 interrupt
}
//////////////////////////////////////////////////////////////

ISR(INT0_vect)
{
  // disable int0 interrupt
  GIMSK &= ~(1 << INT0);
  TCNT0 = 0;
  // ovf = 0xff
  // t = ovf / (f_cpu / prescaler)
  // we want t ~ 0.1
  TCCR0B |= (1 << CS02) | (1 << CS00);  // prescaler  = 1024
  // enable timer0 overflow interrupt
  TIMSK |= (1 << TOIE0);
}
//////////////////////////////////////////////////////////////

void btns_init(void)
{
  BTNS_DDR &= ~((1 << BTNS_INT_DDR_N) | (1 << BTN_ENTER_DDR_N) |
                (1 << BTN_UP_DDR_N) | (1 << BTN_DOWN_DDR_N));
  MCUCR = (1 << ISC01);  // falling edge on INT0 generates interrupt
  GIMSK = (1 << INT0);   // enable INT0 interrupt
}
//////////////////////////////////////////////////////////////

void btn_pressed(void)
{
  if ((BTNS_PIN & (1 << BTN_ENTER_PIN)) == 0) {
    btn_enter_pressed();
  }

  if ((BTNS_PIN & (1 << BTN_UP_PIN)) == 0) {
    btn_up_pressed();
  }

  if ((BTNS_PIN & (1 << BTN_DOWN_PIN)) == 0) {
    btn_down_pressed();
  }
}
//////////////////////////////////////////////////////////////

void rtc_print(const rtc_t *r)
{
  char buff[3] = {0};
  char *s = str_u8_n(r->hour, buff, 3, '0');
  nokia5110_write_str(s);
  nokia5110_write_char(':');
  s = str_u8_n(r->minute, buff, 3, '0');
  nokia5110_write_str(s);
  nokia5110_write_char(':');
  s = str_u8_n(r->second, buff, 3, '0');
  nokia5110_write_str(s);
}
//////////////////////////////////////////////////////////////

void display_current_menu(void)
{
  const char *hdr = "    OPEN    CLOSE";
  nokia5110_gotoXY(0, 0);
  nokia5110_write_str("     ");
  rtc_print(&m_menu.current_time);
  nokia5110_gotoXY(0, 1);
  nokia5110_write_str(hdr);
  for (int i = 0; i < m_menu.valves_len; ++i) {
    nokia5110_gotoXY(0, 2 + i);
    rtc_print(valve_get_open_time(&m_menu.valves[i]));
    nokia5110_write_str(" ");
    rtc_print(valve_get_close_time(&m_menu.valves[i]));
  }
}
//////////////////////////////////////////////////////////////

void btn_enter_pressed(void) {}
//////////////////////////////////////////////////////////////

void btn_up_pressed(void) {}
//////////////////////////////////////////////////////////////

void btn_down_pressed(void) {}
//////////////////////////////////////////////////////////////

int main(void)
{
  static bool lge = false;
  valves_init(&m_menu.valves, &m_menu.valves_len);
  m_menu.current_time.hour = __CT_HOUR;
  m_menu.current_time.minute = __CT_MINUTE;
  m_menu.current_time.second = __CT_SECOND;

  nokia5110_init();
  led_grn_init();
  btns_init();
  timer1_init();

  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  while (2 + 2 != 5) {
    sleep_cpu();
    if (SOFT_INTERRUPTS_REG & SIVF_TICK_PASSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_TICK_PASSED;
      led_grn_turn_on(lge = !lge);
      if (lge) {
        rtc_inc(&m_menu.current_time);
        display_current_menu();
      }
    }

    if (SOFT_INTERRUPTS_REG & SIVF_BTN_PRESSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_BTN_PRESSED;
      btn_pressed();
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////

void led_grn_init(void)
{
  LED_GRN_DDR |= (1 << LED_GRN_DDR_N);
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
