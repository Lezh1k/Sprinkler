// see fuses
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "commons.h"
#include "nokia_5110.h"
#include "rt_clock.h"
#include "valve.h"

#define BTNS_DDR  DDRD
#define BTNS_PIN  PIND
#define BTNS_PORT PORTD

#define BTNS_INT_DDR_N DDD2
#define BTNS_INT_PIN   PIND2
#define BTNS_INT_PORT  PORTD2

#define BTN_ENTER_DDR_N DDD3
#define BTN_ENTER_PIN   PIND3
#define BTN_ENTER_PORT  PORTD3

#define BTN_UP_DDR_N DDD4
#define BTN_UP_PIN   PIND4
#define BTN_UP_PORT  PORTD4

#define BTN_DOWN_DDR_N DDD5
#define BTN_DOWN_PIN   PIND5
#define BTN_DOWN_PORT  PORTD5

#define TICK_DURATION_MS 500

#define F_CPU            16000000
#define TICK_OCRA1_VAL   15625
/* #define F_CPU          4000000 */
/* #define TICK_OCRA1_VAL (15625 / 4) */

static volatile uint8_t SOFT_INTERRUPTS_REG = 0;

enum soft_interrupts_flags {
  SIVF_TICK_PASSED = 1 << 1,
  SIVF_BTN_PRESSED = 1 << 2,
};
//////////////////////////////////////////////////////////////

// buttons
static void btns_init(void);
static void btn_pressed(void);
static void btn_enter_pressed(void);
static void btn_up_pressed(void);
static void btn_down_pressed(void);

// display
static void rtc_print(const rtc_t *r, const uint8_t *ptr_skip);
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

static void display_current_time(uint8_t *ptr_skip);
static void display_valves_settings(uint8_t *ptr_skip);
static void display_current_menu(bool blink_setting);
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
  GIMSK |= (1 << INT0);  // enable INT0 interrupt
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

  // ENABLE PULL UP RESISTORS
  BTNS_PORT |= ((1 << BTNS_INT_PORT) | (1 << BTN_ENTER_PORT) |
                (1 << BTN_UP_PORT) | (1 << BTN_DOWN_PORT));

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

void rtc_print(const rtc_t *r, const uint8_t *ptr_skip)
{
  static char buff[3] = {0};
  char *s;

  for (uint8_t i = 0; i < 3; ++i) {
    s = &r->arr[i] == ptr_skip ? "  " : str_u8_n(r->arr[i], buff, 3, '0');
    nokia5110_write_str(s);
    if (i < 2) {
      nokia5110_write_char(':');
    }
  }
}
//////////////////////////////////////////////////////////////

void display_valves_settings(uint8_t *ptr_skip)
{
  static const char *hdr = "    OPEN    CLOSE";
  nokia5110_gotoXY(0, 1);
  nokia5110_write_str(hdr);
  for (int8_t i = 0; m_valves_state.valves[i]; ++i) {
    valve_t *pv = m_valves_state.valves[i];
    nokia5110_gotoXY(0, 2 + i);
    rtc_print(&pv->open, ptr_skip);
    nokia5110_write_str(" ");
    rtc_print(&pv->close, ptr_skip);
  }
}
//////////////////////////////////////////////////////////////

void display_current_time(uint8_t *ptr_skip)
{
  nokia5110_gotoXY(0, 0);
  nokia5110_write_str("     ");
  rtc_print(&m_valves_state.current_time, ptr_skip);
}
//////////////////////////////////////////////////////////////

void display_current_menu(bool blink_setting)
{
  uint8_t *ptr_skip = NULL;
  if (blink_setting) {
    ptr_skip = ptr_setting();
  }
  display_current_time(ptr_skip);
  display_valves_settings(ptr_skip);
}
//////////////////////////////////////////////////////////////

void check_and_handle_valves_state(void)
{
  for (int8_t i = 0; m_valves_state.valves[i]; ++i) {
    valve_t *v = m_valves_state.valves[i];
    rtc_t *ct = &m_valves_state.current_time;
    if (rtc_eq(ct, &v->open)) {
      valve_open(i);
    }

    if (rtc_eq(ct, &v->close)) {
      valve_close(i);
    }
  }
}
//////////////////////////////////////////////////////////////

void btn_enter_pressed(void)
{
  m_valves_state.is_editing = !m_valves_state.is_editing;
}
//////////////////////////////////////////////////////////////

static const uint8_t time_limits[3] = {24, 60, 60};
uint8_t *ptr_setting(void)
{
  uint8_t rtc_idx = m_valves_state.settings_idx / 3;
  uint8_t setting_idx = m_valves_state.settings_idx % 3;

  rtc_t *ct = &m_valves_state.current_time;
  if (rtc_idx > 0) {
    uint8_t val_idx = (rtc_idx - 1) / 2;  // 1,2 - 0; 3,4 - 1; 5,6 - 2
    ct = &m_valves_state.valves[val_idx]->close;
    if (rtc_idx & 0x01) {
      ct = &m_valves_state.valves[val_idx]->open;
    }
  }
  return &ct->arr[setting_idx];
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
  m_valves_state.current_time.time.hour = __CT_HOUR;
  m_valves_state.current_time.time.minute = __CT_MINUTE;
  m_valves_state.current_time.time.second = __CT_SECOND;
  m_valves_state.is_editing = false;
  m_valves_state.settings_idx = 0;

  timer1_init();
  nokia5110_init();
  btns_init();

  sei();

  display_current_time(NULL);
  display_valves_settings(NULL);

  bool bs = false;
  while (2 + 2 != 5) {
    if (SOFT_INTERRUPTS_REG & SIVF_TICK_PASSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_TICK_PASSED;
      if ((second_passed = !second_passed)) {
        rtc_inc(&m_valves_state.current_time);
        check_and_handle_valves_state();
      }
      display_current_menu((bs = !bs) && m_valves_state.is_editing);
    }

    if (SOFT_INTERRUPTS_REG & SIVF_BTN_PRESSED) {
      SOFT_INTERRUPTS_REG &= ~SIVF_BTN_PRESSED;
      btn_pressed();
      display_current_menu(bs = true);
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////
