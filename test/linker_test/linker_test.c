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


extern int function_in_code0();
extern int function_in_code1();

void something_else() __nonbanked
{
	vdp_print_grp1(10, 12, "something else");
}

void something_else2() __nonbanked
{
	vdp_print_grp1(10, 16, "something more");
}


void main()
{
	log_w("we are running");
	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	vdp_print_grp1(10, 10, "Hello MegaROM");

	function_in_code0();

	function_in_code1();

	something_else2();

	something_else();

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
