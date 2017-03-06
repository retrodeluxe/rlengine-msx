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


extern const unsigned char testmap_cmpr_dict;

void main()
{

	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	vdp_print_grp1(10, 10, "Hello 48K ROM");

	log_e("pointer to data %d\n", testmap_cmpr_dict);

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
