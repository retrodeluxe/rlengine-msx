/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"

void putchar(char c);
void puts(char *s);

int main(char** argv, int argc)
{
  puts("Hello MSXDOS\n");

  vdp_set_mode(MODE_GRP1);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(0);

  vdp_puts(10, 10, "Hello MSX");

  do {
  } while (sys_get_key(8) & 1);

  return 0;
}

void putchar(char c) {
    __asm
    ld		e,4(ix)
    ld		c,#2
    call	5
  __endasm;
}

void puts(char *s) {
	while (*s != 0) {
		putchar(*s);
		s++;
	}
}
