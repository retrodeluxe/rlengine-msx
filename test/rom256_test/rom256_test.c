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

extern const char test[];
extern const char test2[];

void main() __nonbanked
{

	log_w("we are running");
	vdp_set_mode(MODE_GRP1);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

	vdp_puts(10, 10, "Hello MegaROM");
	ascii8_set_data(4);
	vdp_puts(10, 11, test);
	ascii8_set_data(5);
	vdp_puts(10, 12, test2);

	do {
	} while (sys_get_keyrow(8) & 1);

	sys_reboot();
}
