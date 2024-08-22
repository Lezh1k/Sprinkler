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

#define F_CPU 16000000
/* #define F_CPU 4000000 */

static volatile struct {
  bool SIR_timer1_compa;
  bool SIR_btn_pressed;
} SOFT_INTERRUPTS_REG;

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
#define DUMMY_VALVE_IDX -1
typedef struct settings_idx {
  int8_t valve_idx;
  int8_t rtc_idx;
  int8_t rtc_part_idx;
} settings_idx_t;

static void settings_idx_inc(settings_idx_t *sidx, int8_t valves_n);
static void settings_idx_dec(settings_idx_t *sidx, int8_t valves_n);

typedef struct valves_state {
  // data
  rtc_t current_time;
  valve_t **valves;
  // aux
  settings_idx_t settings_idx;
  bool is_editing;
} valves_state_t;
static valves_state_t m_valves_state = {0};

static void display_current_time(void);
static void display_current_menu(void);
static void check_and_handle_valves_state(void);
//////////////////////////////////////////////////////////////

// timer 1
ISR(TIMER1_COMPA_vect)
{
  static volatile bool second_passed = false;
  if ((second_passed = !second_passed)) {
    rtc_inc(&m_valves_state.current_time);
  }
  SOFT_INTERRUPTS_REG.SIR_timer1_compa = true;
}

static void timer1_init(void)
{
  TCCR1B |= (1 << CS11) | (1 << CS10);  // 64 prescaler
  // we have 8 prescaler fuse bits and 64 prescaler for timer
  // so 1 second on timer is (F_CPU / 8 / 64)
  // we want interrupt each 0.5 seconds
  OCR1A = (F_CPU / 8 / 64 / 2);
  TCCR1B |= (1 << WGM12);  // CTC without toggling output
  TCNT1 = 0;
  SOFT_INTERRUPTS_REG.SIR_timer1_compa = false;
  TIMSK |= (1 << OCIE1A);  // timer1_compa_int_enable
}
//////////////////////////////////////////////////////////////

// settings idx
void settings_idx_inc(settings_idx_t *sidx, int8_t valves_n)
{
  uint8_t rtc_n = sidx->valve_idx == DUMMY_VALVE_IDX ? 1 : 2;
  uint8_t cf = ++sidx->rtc_part_idx == 3;
  sidx->rtc_idx += cf;
  cf = sidx->rtc_idx == rtc_n;
  sidx->valve_idx += cf;

  if (sidx->rtc_part_idx == 3)
    sidx->rtc_part_idx = 0;
  if (sidx->rtc_idx == rtc_n)
    sidx->rtc_idx = 0;
  if (sidx->valve_idx == valves_n)
    sidx->valve_idx = DUMMY_VALVE_IDX;
}

void settings_idx_dec(settings_idx_t *sidx, int8_t valves_n)
{
  uint8_t cf = --sidx->rtc_part_idx == -1;
  sidx->rtc_idx -= cf;
  cf = sidx->rtc_idx == -1;
  sidx->valve_idx -= cf;

  /* uint8_t rtc_n = 2 - (sidx->valve_idx == DUMMY_VALVE_IDX); */
  uint8_t rtc_n = sidx->valve_idx == DUMMY_VALVE_IDX ? 1 : 2;
  if (sidx->rtc_part_idx == -1)
    sidx->rtc_part_idx = 2;
  if (sidx->rtc_idx == -1)
    sidx->rtc_idx = rtc_n;
  if (sidx->valve_idx == DUMMY_VALVE_IDX - 1)
    sidx->valve_idx = valves_n - 1;
}
//////////////////////////////////////////////////////////////

// btns
ISR(TIMER0_OVF_vect)
{
  // disable timer0_ovf interrupt
  TIMSK &= ~(1 << TOIE0);
  if ((BTNS_PIN & (1 << BTNS_INT_PIN)) == 0) {
    SOFT_INTERRUPTS_REG.SIR_btn_pressed = true;
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
#if (F_CPU == 16000000)
  TCCR0B |= (1 << CS02) | (1 << CS00);  // prescaler  = 1024
#elif (F_CPU == 40000)
  TCCR0B |= (1 << CS02);  // prescaler  = 256
#endif
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

void rtc_print(const rtc_t *r)
{
  static char buff[3] = {0};
  char *s;
  for (uint8_t i = 0; i < 3; ++i) {
    s = str_u8_n(r->arr[i], buff, 3, '0');
    nokia5110_write_str(s);
    if (i < 2) {
      nokia5110_write_char(':');
    }
  }
}
//////////////////////////////////////////////////////////////

#define CURRNET_TIME_X_OFFSET 5
void display_current_time(void)
{
  nokia5110_gotoXY(CURRNET_TIME_X_OFFSET, 0);
  rtc_print(&m_valves_state.current_time);
}
//////////////////////////////////////////////////////////////

void display_current_menu(void)
{
  static const char *hdr = "OPEN    CLOSE";
  display_current_time();
  nokia5110_gotoXY(4, 1);
  nokia5110_write_str(hdr);
  for (int8_t i = 0; m_valves_state.valves[i]; ++i) {
    valve_t *pv = m_valves_state.valves[i];
    nokia5110_gotoXY(0, 2 + i);
    rtc_print(&pv->schedule.strct.open);
    nokia5110_write_str(" ");
    rtc_print(&pv->schedule.strct.close);
  }
}
//////////////////////////////////////////////////////////////

void check_and_handle_valves_state(void)
{
  rtc_t *ct = &m_valves_state.current_time;
  for (valve_t **ppv = m_valves_state.valves; *ppv; ++ppv) {
    valve_t *v = *ppv;
    if (rtc_eq(ct, &v->schedule.strct.open)) {
      valve_open(v);
    }

    if (rtc_eq(ct, &v->schedule.strct.close)) {
      valve_close(v);
    }
  }
}
//////////////////////////////////////////////////////////////

static rtc_t *current_rtc_ptr(void)
{
  settings_idx_t *si = &m_valves_state.settings_idx;
  if (si->valve_idx == DUMMY_VALVE_IDX)
    return &m_valves_state.current_time;
  return &m_valves_state.valves[si->valve_idx]->schedule.arr[si->rtc_idx];
}
//////////////////////////////////////////////////////////////

void btn_enter_pressed(void)
{
  m_valves_state.is_editing = !m_valves_state.is_editing;
}
//////////////////////////////////////////////////////////////

void btn_up_pressed(void)
{
  if (!m_valves_state.is_editing) {
    settings_idx_inc(&m_valves_state.settings_idx, 3);
    return;
  }
  rtc_part_inc(current_rtc_ptr(), m_valves_state.settings_idx.rtc_part_idx);
}
//////////////////////////////////////////////////////////////

void btn_down_pressed(void)
{
  if (!m_valves_state.is_editing) {
    settings_idx_dec(&m_valves_state.settings_idx, 3);
    return;
  }
  rtc_part_dec(current_rtc_ptr(), m_valves_state.settings_idx.rtc_part_idx);
}
//////////////////////////////////////////////////////////////

static void redraw_current_setting(bool blink)
{
  static char buff[3] = {0};
  settings_idx_t *si = &m_valves_state.settings_idx;
  uint8_t x_offset = CURRNET_TIME_X_OFFSET;
  uint8_t y = 0;
  if (si->valve_idx != DUMMY_VALVE_IDX) {
    y = si->valve_idx + 2;  // starting from 2nd line
    x_offset = 0;
  }
  uint8_t x = si->rtc_idx * 9 + si->rtc_part_idx * 3 + x_offset;

  rtc_t *ct = current_rtc_ptr();
  nokia5110_gotoXY(x, y);
  char *s = blink ? "  " : str_u8_n(ct->arr[si->rtc_part_idx], buff, 3, '0');
  nokia5110_write_str(s);
}
//////////////////////////////////////////////////////////////

int main(void)
{
  valves_init(&m_valves_state.valves);
  m_valves_state.current_time.time.hour = __CT_HOUR;
  m_valves_state.current_time.time.minute = __CT_MINUTE;
  m_valves_state.current_time.time.second = __CT_SECOND;
  m_valves_state.is_editing = false;
  m_valves_state.settings_idx.valve_idx = DUMMY_VALVE_IDX;
  m_valves_state.settings_idx.rtc_idx =
      m_valves_state.settings_idx.rtc_part_idx = 0;

  timer1_init();
  btns_init();
  sei();

  nokia5110_init();
  display_current_menu();

  bool blink = false;
  while (2 + 2 != 5) {
    if (SOFT_INTERRUPTS_REG.SIR_timer1_compa) {
      SOFT_INTERRUPTS_REG.SIR_timer1_compa = false;
      check_and_handle_valves_state();
      display_current_time();
      redraw_current_setting((blink = !blink) && m_valves_state.is_editing);
    }

    if (SOFT_INTERRUPTS_REG.SIR_btn_pressed) {
      SOFT_INTERRUPTS_REG.SIR_btn_pressed = false;
      btn_pressed();
      redraw_current_setting(blink = true);
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////
