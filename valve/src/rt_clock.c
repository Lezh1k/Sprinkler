#include "rt_clock.h"

enum {
  TL_HOUR = 24,
  TL_MIN = 60,
  TL_SEC = 60,
};

void rtc_inc(rtc_t *rtc)
{
  int8_t cf = ++rtc->time.second == TL_SEC;
  rtc->time.minute += cf;
  cf = rtc->time.minute == TL_MIN;
  rtc->time.hour += cf;

  if (rtc->time.second == TL_SEC)
    rtc->time.second = 0;
  if (rtc->time.minute == TL_MIN)
    rtc->time.minute = 0;
  if (rtc->time.hour == TL_HOUR)
    rtc->time.hour = 0;
}
//////////////////////////////////////////////////////////////

static const uint8_t time_limits[3] = {TL_HOUR, TL_MIN, TL_SEC};
void rtc_part_inc(rtc_t *rtc, int8_t idx)
{
  if (++rtc->arr[idx] == time_limits[idx])
    rtc->arr[idx] = 0;
};
//////////////////////////////////////////////////////////////

void rtc_part_dec(rtc_t *rtc, int8_t idx)
{
  if (rtc->arr[idx]-- == 0)
    rtc->arr[idx] = time_limits[idx] - 1;
};
//////////////////////////////////////////////////////////////

bool rtc_eq(const rtc_t *l, const rtc_t *r)
{
  return l->time.hour == r->time.hour && l->time.minute == r->time.minute &&
         l->time.second == r->time.second;
}
//////////////////////////////////////////////////////////////
