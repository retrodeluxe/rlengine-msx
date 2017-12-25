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

//extern const unsigned char testmap_cmpr_dict;
extern const char test[];
extern const char test2[];

extern void sys_set_ascii_page3(char page);

void main()
{

	log_w("we are running");
	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	vdp_print_grp1(10, 10, "Hello MegaROM");
	sys_set_ascii_page3(4);
	vdp_print_grp1(10, 11, test);
	sys_set_ascii_page3(5);
	vdp_print_grp1(10, 12, test2);

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
