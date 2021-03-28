/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
#include "ascii8.h"

extern int function_in_code0();
extern int function_in_code1(uint8_t val);
extern const char caca1[];

void something_else(uint16_t val) __nonbanked;
void something_else2(uint16_t val); // the modifier is screwing up parameters passing
void something_else3() __nonbanked;

void main() __nonbanked
{
  log_w("we are running\n");
  vdp_set_mode(MODE_GRP1);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(0);

  vdp_puts(10, 10, "Hello MegaROM");

  ascii8_get_page(function_in_code0);
  log_e("read page %d\n", ascii8_page);

  ascii8_get_page(function_in_code1);
  log_e("read page %d\n", ascii8_page);

  ascii8_get_page(caca1);
  log_e("read page %d\n", ascii8_page);

  function_in_code0();

  function_in_code1(77);

  something_else2(88);

  something_else(66);

  something_else3();

  do {
  } while (sys_get_key(8) & 1);

  sys_reboot();
}

void something_else(uint16_t val) __nonbanked
{
    log_e("sthing else received val %d\n", val);
    vdp_puts(10, 12, "something else");
}

void something_else2(uint16_t val)
{
  log_e("sthing else2 received val %d\n", val);
  vdp_puts(10, 16, "something more");
}

void something_else3() __nonbanked
{
  log_e("nonbaked no params\n");
}
