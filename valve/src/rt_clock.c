#include "rt_clock.h"

#include <stdint.h>

void rtc_inc(rtc_t *rtc)
{
  int8_t cr = ++rtc->second == 60;
  rtc->minute += cr;
  cr = rtc->minute == 60;
  rtc->hour += cr;

  if (rtc->second == 60)
    rtc->second = 0;
  if (rtc->minute == 60)
    rtc->minute = 0;
  if (rtc->hour == 24)
    rtc->hour = 0;
}
