#ifndef RT_CLOCK_H
#define RT_CLOCK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct rtc {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} rtc_t;

void rtc_inc(rtc_t *rtc);
bool rtc_eq(const rtc_t *l, const rtc_t *r);

//////////////////////////////////////////////////////////////

#endif
