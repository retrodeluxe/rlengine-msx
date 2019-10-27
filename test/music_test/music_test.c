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
	vdp_clear_grp1(0);


	vdp_print_grp1(0, 0, "Music Test");

	sys_memcpy(NoteTable, NT, 96*2);
	PT3Init((unsigned int) SONG00 ,0);

	sys_irq_init();
	sys_proc_register(play_music);
	do {
		__asm halt __endasm;
		PT3Decode();
		log_e("0");
	} while (sys_get_key(8) & 1);

	PT3Mute();

	sys_reboot();
}

void play_music()
{
	PT3PlayAY();
}
