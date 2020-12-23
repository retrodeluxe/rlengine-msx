/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"


extern int function_in_code0();
extern int function_in_code1();
extern const char caca1[];

void something_else() __nonbanked;
void something_else2() __nonbanked;

void main() __nonbanked
{
  log_w("we are running");
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

  function_in_code1();

  something_else2();

  something_else();

  do {
  } while (sys_get_key(8) & 1);

  sys_reboot();
}

void something_else() __nonbanked
{
  vdp_puts(10, 12, "something else");
}

void something_else2() __nonbanked
{
  vdp_puts(10, 16, "something more");
}
