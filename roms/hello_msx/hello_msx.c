/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"

void main()
{
	vdp_set_mode(MODE_GRP1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear(0);

	vdp_puts(10, 10, "Hello MSX");

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
