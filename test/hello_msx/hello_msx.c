/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"

void main()
{
	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	vdp_print_grp1(10, 10, "Hello MSX");

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}