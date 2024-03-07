#ifndef RT_CLOCK_H
#define RT_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

typedef union rtc {
  struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
  } time;
  uint8_t arr[3];
} rtc_t;

void rtc_inc(rtc_t *rtc);
bool rtc_eq(const rtc_t *l, const rtc_t *r);

//////////////////////////////////////////////////////////////

#endif
