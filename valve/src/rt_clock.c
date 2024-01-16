#include <avr/builtins.h>
#include <stdint.h>

void rtc_init(uint32_t secs)
{
  uint8_t h = secs / 3600;
  secs -= h * 3600;
  uint8_t m = secs / 60;
  secs -= m * 60;
  uint8_t s = secs;
  (void)s;
}
//////////////////////////////////////////////////////////////

void rtc_second_passed(void)
{
  __asm__("nop");
}
//////////////////////////////////////////////////////////////
