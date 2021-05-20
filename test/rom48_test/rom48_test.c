/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"

extern const char test_string;

extern void sys_set_rom();
extern void sys_set_bios();

char local_string[50];

void main()
{
  vdp_set_mode(MODE_GRP1);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(0);

  /*
   * Access to ROM in lower addresses defines a critical section
   * with interrupts disabled.
   *
   * Inside the critical section only data copy to RAM is allowed,
   * as anything that enables interrupts would result in a hang.
   */
  sys_set_rom();
  {
     sys_memcpy(local_string, &test_string, 15);
  }
  sys_set_bios();

  vdp_puts(10, 13, local_string);

  do {
  } while (sys_get_keyrow(8) & 1);

  sys_reboot();
}
