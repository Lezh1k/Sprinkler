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

#define BTNS_DDR DDRD
#define BTNS_PIN PIND
#define BTNS_PORT PORTD

#define BTNS_INT_DDR_N DDD2
#define BTNS_INT_PIN PIND2
#define BTNS_INT_PORT PORTD2

#define BTN_ENTER_DDR_N DDD3
#define BTN_ENTER_PIN PIND3
#define BTN_ENTER_PORT PORTD3

#define BTN_UP_DDR_N DDD4
#define BTN_UP_PIN PIND4
#define BTN_UP_PORT PORTD4

#define BTN_DOWN_DDR_N DDD5
#define BTN_DOWN_PIN PIND5
#define BTN_DOWN_PORT PORTD5

// #define F_CPU 16000000
// #define F_CPU 8000000
#define F_CPU 24000000

static volatile struct {
  bool SIR_timer1_tick;
  bool SIR_btn_pressed;
} SOFT_INTERRUPTS_REG = {0};

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
#define CONTROLLED_VALVES_N 3
typedef struct settings_idx {
  int8_t valve_idx;
  int8_t rtc_idx;
  int8_t rtc_part_idx;
} settings_idx_t;

static void settings_idx_inc(settings_idx_t *sidx, int8_t valves_n);
static void settings_idx_dec(settings_idx_t *sidx, int8_t valves_n);

typedef enum controller_mode {
  CONTROLLER_MODE_NORMAL = 0,
  CONTROLLER_MODE_SETTINGS,
} controller_mode_t;

typedef enum blink_state {
  BLINK_STABLE_VISIBLE = 0,
  BLINK_SINGLE_HIDDEN,
  BLINK_CONTINUOUS_VISIBLE,
  BLINK_CONTINUOUS_HIDDEN,
} blink_state_t;

typedef struct valves_state {
  // data
  rtc_t current_time;
  valve_t **valves;
  // aux
  settings_idx_t settings_idx;
  controller_mode_t mode;
  blink_state_t blink_state;
} valves_state_t;
static valves_state_t m_valves_state = {0};

static void display_current_time(void);
static void display_current_menu(void);
static void display_selected_setting(bool hidden);
static void check_and_handle_valves_state(void);
static void controller_toggle_mode(void);
static void controller_move_selection(bool forward);
static void controller_change_current_setting(bool increment);
static void controller_on_half_second_tick(void);
//////////////////////////////////////////////////////////////

// timer 1
ISR(TIMER1_COMPA_vect) { SOFT_INTERRUPTS_REG.SIR_timer1_tick = true; }

static void timer1_init(void) {
  TCCR1B |= (1 << CS12) |      // 256 prescaler
            (1 << WGM12);      // CTC without toggling output
  OCR1A = ((F_CPU / 256) / 2); // 0.5s
  TCNT1 = 0;
  TIMSK |= (1 << OCIE1A); // timer1_compa_int_enable
}
//////////////////////////////////////////////////////////////

// settings idx
static void settings_idx_inc(settings_idx_t *sidx, int8_t valves_n) {
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

static void settings_idx_dec(settings_idx_t *sidx, int8_t valves_n) {
  uint8_t cf = --sidx->rtc_part_idx == -1;
  sidx->rtc_idx -= cf;
  cf = sidx->rtc_idx == -1;
  sidx->valve_idx -= cf;
  uint8_t rtc_n = sidx->valve_idx == DUMMY_VALVE_IDX ? 1 : 2;

  if (sidx->rtc_part_idx == -1)
    sidx->rtc_part_idx = 2;
  if (sidx->rtc_idx == -1)
    sidx->rtc_idx = rtc_n - 1;
  if (sidx->valve_idx == (DUMMY_VALVE_IDX - 1))
    sidx->valve_idx = valves_n - 1;
}
//////////////////////////////////////////////////////////////

// btns
ISR(TIMER0_OVF_vect) {
  // disable timer0_ovf interrupt
  TIMSK &= ~(1 << TOIE0);
  if ((BTNS_PIN & (1 << BTNS_INT_PIN)) == 0) {
    SOFT_INTERRUPTS_REG.SIR_btn_pressed = true;
  }
  GIMSK |= (1 << INT0); // enable INT0 interrupt
}
//////////////////////////////////////////////////////////////

ISR(INT0_vect) {
  // disable int0 interrupt
  GIMSK &= ~(1 << INT0);
  TCNT0 = 0;
  // ovf = 0xff
  // t = ovf / (f_cpu / prescaler)
  // we want t ~ 0.1
#if (F_CPU == 16000000 || F_CPU == 24000000)
  TCCR0B |= (1 << CS02) | (1 << CS00); // prescaler  = 1024
#elif (F_CPU == 4000000 || F_CPU == 8000000)
  TCCR0B |= (1 << CS02); // prescaler  = 256
#else
#error Please specify timer0 prescaler for F_CPU
#endif
  // enable timer0 overflow interrupt
  TIMSK |= (1 << TOIE0);
}
//////////////////////////////////////////////////////////////

static void btns_init(void) {
  BTNS_DDR &= ~((1 << BTNS_INT_DDR_N) | (1 << BTN_ENTER_DDR_N) |
                (1 << BTN_UP_DDR_N) | (1 << BTN_DOWN_DDR_N));

  // ENABLE PULL UP RESISTORS
  BTNS_PORT |= ((1 << BTNS_INT_PORT) | (1 << BTN_ENTER_PORT) |
                (1 << BTN_UP_PORT) | (1 << BTN_DOWN_PORT));

  MCUCR = (1 << ISC01); // falling edge on INT0 generates interrupt
  GIMSK = (1 << INT0);  // enable INT0 interrupt
}
//////////////////////////////////////////////////////////////

static void btn_pressed(void) {
  if ((BTNS_PIN & (1 << BTN_ENTER_PIN)) == 0) {
    btn_enter_pressed();
    return;
  }

  if ((BTNS_PIN & (1 << BTN_UP_PIN)) == 0) {
    btn_up_pressed();
    return;
  }

  if ((BTNS_PIN & (1 << BTN_DOWN_PIN)) == 0) {
    btn_down_pressed();
    return;
  }
}
//////////////////////////////////////////////////////////////

static void rtc_print(const rtc_t *r) {
  static char buff[3] = {0};
  char *s;
  for (uint8_t i = 0; i < 2; ++i) {
    s = str_u8_n(r->arr[i], buff, 3);
    nokia5110_write_str(s);
    nokia5110_write_char(':');
  }
  s = str_u8_n(r->arr[2], buff, 3);
  nokia5110_write_str(s);
}
//////////////////////////////////////////////////////////////

#define CURRENT_TIME_X_OFFSET 5
static void display_current_time(void) {
  nokia5110_gotoXY(CURRENT_TIME_X_OFFSET, 0);
  rtc_print(&m_valves_state.current_time);
}
//////////////////////////////////////////////////////////////

static void display_current_menu(void) {
  static const char *hdr = "OPEN    CLOSE";
  display_current_time();
  nokia5110_gotoXY(4, 1);
  nokia5110_write_str(hdr);
  for (int8_t i = 0; m_valves_state.valves[i]; ++i) {
    valve_t *pv = m_valves_state.valves[i];
    nokia5110_gotoXY(0, 2 + i);
    rtc_print(&pv->schedule.strct.open);
    nokia5110_write_char(' ');
    rtc_print(&pv->schedule.strct.close);
  }
}
//////////////////////////////////////////////////////////////

static void check_and_handle_valves_state(void) {
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

static rtc_t *current_rtc_ptr(void) {
  settings_idx_t *si = &m_valves_state.settings_idx;
  if (si->valve_idx == DUMMY_VALVE_IDX)
    return &m_valves_state.current_time;
  return &m_valves_state.valves[si->valve_idx]->schedule.arr[si->rtc_idx];
}
//////////////////////////////////////////////////////////////

static void btn_enter_pressed(void) {
  controller_toggle_mode();
}
//////////////////////////////////////////////////////////////

static void btn_up_pressed(void) {
  if (m_valves_state.mode == CONTROLLER_MODE_NORMAL) {
    controller_move_selection(true);
    return;
  }

  controller_change_current_setting(true);
}
//////////////////////////////////////////////////////////////

static void btn_down_pressed(void) {
  if (m_valves_state.mode == CONTROLLER_MODE_NORMAL) {
    controller_move_selection(false);
    return;
  }

  controller_change_current_setting(false);
}
//////////////////////////////////////////////////////////////

static void display_selected_setting(bool hidden) {
  static char buff[3] = {0};
  settings_idx_t *si = &m_valves_state.settings_idx;
  uint8_t x_offset = CURRENT_TIME_X_OFFSET;
  uint8_t y = 0;
  if (si->valve_idx != DUMMY_VALVE_IDX) {
    y = si->valve_idx + 2;
    x_offset = 0;
  }
  uint8_t x = si->rtc_idx * 9 + si->rtc_part_idx * 3 + x_offset;

  rtc_t *ct = current_rtc_ptr();
  nokia5110_gotoXY(x, y);
  char *s = hidden ? "  " : str_u8_n(ct->arr[si->rtc_part_idx], buff, 3);
  nokia5110_write_str(s);
}
//////////////////////////////////////////////////////////////

static void controller_toggle_mode(void) {
  if (m_valves_state.mode == CONTROLLER_MODE_NORMAL) {
    m_valves_state.mode = CONTROLLER_MODE_SETTINGS;
    m_valves_state.blink_state = BLINK_CONTINUOUS_HIDDEN;
    display_selected_setting(true);
    return;
  }

  m_valves_state.mode = CONTROLLER_MODE_NORMAL;
  m_valves_state.blink_state = BLINK_STABLE_VISIBLE;
  display_selected_setting(false);
}
//////////////////////////////////////////////////////////////

static void controller_move_selection(bool forward) {
  display_selected_setting(false);
  if (forward) {
    settings_idx_inc(&m_valves_state.settings_idx, CONTROLLED_VALVES_N);
  } else {
    settings_idx_dec(&m_valves_state.settings_idx, CONTROLLED_VALVES_N);
  }
  m_valves_state.blink_state = BLINK_SINGLE_HIDDEN;
  display_selected_setting(true);
}
//////////////////////////////////////////////////////////////

static void controller_change_current_setting(bool increment) {
  rtc_t *rtc = current_rtc_ptr();
  int8_t rtc_part_idx = m_valves_state.settings_idx.rtc_part_idx;

  if (increment) {
    rtc_part_inc(rtc, rtc_part_idx);
  } else {
    rtc_part_dec(rtc, rtc_part_idx);
  }

  if (m_valves_state.blink_state == BLINK_CONTINUOUS_HIDDEN) {
    display_selected_setting(true);
    return;
  }

  display_selected_setting(false);
}
//////////////////////////////////////////////////////////////

static void controller_on_half_second_tick(void) {
  switch (m_valves_state.blink_state) {
  case BLINK_STABLE_VISIBLE:
    break;
  case BLINK_SINGLE_HIDDEN:
    m_valves_state.blink_state = BLINK_STABLE_VISIBLE;
    display_selected_setting(false);
    break;
  case BLINK_CONTINUOUS_VISIBLE:
    m_valves_state.blink_state = BLINK_CONTINUOUS_HIDDEN;
    display_selected_setting(true);
    break;
  case BLINK_CONTINUOUS_HIDDEN:
    m_valves_state.blink_state = BLINK_CONTINUOUS_VISIBLE;
    display_selected_setting(false);
    break;
  }
}
//////////////////////////////////////////////////////////////

int main(void) {
  valves_init(&m_valves_state.valves);
  m_valves_state.current_time.time.hour = __CT_HOUR;
  m_valves_state.current_time.time.minute = __CT_MINUTE;
  m_valves_state.current_time.time.second = __CT_SECOND;
  m_valves_state.mode = CONTROLLER_MODE_NORMAL;
  m_valves_state.blink_state = BLINK_STABLE_VISIBLE;
  m_valves_state.settings_idx.valve_idx = DUMMY_VALVE_IDX;
  m_valves_state.settings_idx.rtc_idx =
      m_valves_state.settings_idx.rtc_part_idx = 0;

  timer1_init();
  btns_init();
  sei();

  nokia5110_init();
  display_current_menu();

  bool second_passed = false;
  while (2 + 2 != 5) {
    if (SOFT_INTERRUPTS_REG.SIR_timer1_tick) {
      SOFT_INTERRUPTS_REG.SIR_timer1_tick = false;
      if ((second_passed = !second_passed)) {
        rtc_inc(&m_valves_state.current_time);
        display_current_time();
        check_and_handle_valves_state();
      }
      controller_on_half_second_tick();
    }

    if (SOFT_INTERRUPTS_REG.SIR_btn_pressed) {
      SOFT_INTERRUPTS_REG.SIR_btn_pressed = false;
      btn_pressed();
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////
