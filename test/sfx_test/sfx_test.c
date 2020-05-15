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
#include "sfx.h"
#include "pt3.h"
#include "pt3_nt2.h"

#include "gen/prayerofhope_song.h"
#include "gen/sfx.h"

void play_music();

void main()
{
	uint8_t effect, key;

	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	vdp_print_grp1(0, 0, "sfx test");
	vdp_print_grp1(0, 1, "press zero to seven");

	pt3_init_notes(NT);
	pt3_init(prayerofhope_song_pt3,1);

	sfx_setup(sfx_afb);

	sys_irq_init();
	sys_irq_register(play_music);


	effect = 0;
	for (;;) {
		sys_wait_vsync();

		key = sys_get_char();

		if (key > '/' && key <= '8') {
			effect = key - 48;
			sfx_play_effect(effect ,0);
			log_e("playing effect %d\n", effect);
		}
	}
	pt3_mute();
	sys_reboot();
}

void play_music()
{
	pt3_play();
	pt3_decode();
	sfx_play();
}
