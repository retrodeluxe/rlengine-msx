/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
#include "psg.h"

void main()
{
	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);


	vdp_print_grp1(0, 0, "Music Test");
	
	psg_write(7,0x3E);
	psg_write(8,10);
	psg_write(0,0xFE);
	psg_write(1,0);

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
