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

#include "pt3.h"
#include "pt3_nt2.h"
#include "song.h"

void play_music();

void main()
{
	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear(0);


	vdp_puts(0, 0, "Music Test");

	pt3_init_notes(NT);
	pt3_init(SONG00 ,0);

	sys_irq_init();
	sys_irq_register(play_music);
	do {
		sys_wait_vsync();
	} while (sys_get_key(8) & 1);

	pt3_mute();

	sys_reboot();
}

void play_music()
{
	pt3_decode();
	pt3_play();
}
