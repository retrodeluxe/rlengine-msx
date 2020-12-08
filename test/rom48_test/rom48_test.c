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
extern const char test_string;

char local_string[50];

void main()
{

	vdp_set_mode(MODE_GRP1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear(0);

	sys_set_rom();
	vdp_puts(10, 10, "ROM enabled:");
	vdp_puts(10, 11, &test_string);
	sys_memcpy(local_string, &test_string, 15);
	sys_set_bios();

	vdp_puts(10, 12, "BIOS enabled:");
	vdp_puts(10, 13, local_string);

	do {
	} while (sys_get_key(8) & 1);

	sys_reboot();
}
