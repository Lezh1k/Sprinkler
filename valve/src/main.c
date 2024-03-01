// see fuses
#include "rt_clock.h"
#define F_CPU (16000000 / 8)

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
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

// menu.
// (3 valves * 2 + current time) * 3
#define MENU_MAX_SETTINGS_N 21
typedef struct valves_state {
  // data
  rtc_t current_time;
  valve_t **valves;

  // aux
  uint8_t settings_idx;
  bool is_editing;
} valves_state_t;
static valves_state_t m_valves_state = {0};

static void display_current_menu(void);
static uint8_t *ptr_setting(void);
static void check_and_handle_valves_state(void);
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
  static char buff[3] = {0};
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
  static const char *hdr = "    OPEN    CLOSE";
  nokia5110_gotoXY(0, 0);
  nokia5110_write_str("     ");
  rtc_print(&m_valves_state.current_time);
  nokia5110_gotoXY(0, 1);
  nokia5110_write_str(hdr);
  uint8_t i = 0;
  for (valve_t **v = m_valves_state.valves; *v; ++v, ++i) {
    valve_t *pv = *v;
    nokia5110_gotoXY(0, 2 + i);
    rtc_print(&pv->open);
    nokia5110_write_str(" ");
    rtc_print(&pv->close);
  }
}
//////////////////////////////////////////////////////////////

void check_and_handle_valves_state(void) {
  
}
//////////////////////////////////////////////////////////////

void btn_enter_pressed(void)
{
  m_valves_state.is_editing = !m_valves_state.is_editing;
  led_grn_turn_on(m_valves_state.is_editing);
}
//////////////////////////////////////////////////////////////

static const uint8_t time_limits[3] = {24, 60, 60};
uint8_t *ptr_setting(void)
{
  uint8_t rtc_idx = m_valves_state.settings_idx / 3;
  uint8_t setting_idx = m_valves_state.settings_idx % 3;

  rtc_t *rtc = &m_valves_state.current_time;
  if (rtc_idx > 0) {
    uint8_t val_idx = (rtc_idx - 1) / 2;  // 1,2 - 0; 3,4 - 1; 5,6 - 2
    rtc = &m_valves_state.valves[val_idx]->close;
    if (rtc_idx & 0x01) {
      rtc = &m_valves_state.valves[val_idx]->open;
    }
  }

  switch (setting_idx) {
    case 0:
      return &rtc->hour;
    case 1:
      return &rtc->minute;
    case 2:
      return &rtc->second;
  }
  // we will never be here
  return NULL;
}
//////////////////////////////////////////////////////////////

void btn_up_pressed(void)
{
  if (!m_valves_state.is_editing) {
    m_valves_state.settings_idx =
        (m_valves_state.settings_idx + 1) % MENU_MAX_SETTINGS_N;
    return;
  }

  uint8_t tl = time_limits[m_valves_state.settings_idx % 3];
  uint8_t *ps = ptr_setting();
  *ps = (*ps + 1) % tl;
}
//////////////////////////////////////////////////////////////

void btn_down_pressed(void)
{
  if (!m_valves_state.is_editing) {
    m_valves_state.settings_idx =
        (m_valves_state.settings_idx + MENU_MAX_SETTINGS_N - 1) %
        MENU_MAX_SETTINGS_N;
    return;
  }

  uint8_t tl = time_limits[m_valves_state.settings_idx % 3];
  uint8_t *ps = ptr_setting();
  *ps = (*ps + tl - 1) % tl;
}
//////////////////////////////////////////////////////////////

int main(void)
{
  bool second_passed = false;
  valves_init(&m_valves_state.valves);
  m_valves_state.current_time.hour = __CT_HOUR;
  m_valves_state.current_time.minute = __CT_MINUTE;
  m_valves_state.current_time.second = __CT_SECOND;
  m_valves_state.is_editing = false;
  m_valves_state.settings_idx = 0;

  timer1_init();
  nokia5110_init();
  led_grn_init();
  btns_init();

  sei();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  while (2 + 2 != 5) {
    sleep_cpu();
    if (SOFT_INTERRUPTS_REG & SIVF_TICK_PASSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_TICK_PASSED;
      if ((second_passed = !second_passed)) {
        rtc_inc(&m_valves_state.current_time);
        display_current_menu();
      }
    }

    if (SOFT_INTERRUPTS_REG & SIVF_BTN_PRESSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_BTN_PRESSED;
      btn_pressed();
      display_current_menu();
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
