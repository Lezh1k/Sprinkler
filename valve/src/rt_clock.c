#include "rt_clock.h"

#include <stdint.h>

void rtc_inc(rtc_t *rtc)
{
  int8_t cr = ++rtc->time.second == 60;
  rtc->time.minute += cr;
  cr = rtc->time.minute == 60;
  rtc->time.hour += cr;

  if (rtc->time.second == 60)
    rtc->time.second = 0;
  if (rtc->time.minute == 60)
    rtc->time.minute = 0;
  if (rtc->time.hour == 24)
    rtc->time.hour = 0;
}
//////////////////////////////////////////////////////////////

bool rtc_eq(const rtc_t *l, const rtc_t *r)
{
  return l->time.hour == r->time.hour && l->time.minute == r->time.minute &&
         l->time.second == r->time.second;
}
//////////////////////////////////////////////////////////////
