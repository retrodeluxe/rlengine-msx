/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "vdp.h"
#include "sprite.h"
#include "gen/spr_test.h"
#include <stdlib.h>

/**
 * Global data is placed in 0xC000 (RAM page 2) in 32K roms by default
 */
struct spr_sprite_pattern_set bee_patt;
struct spr_sprite_pattern_set rat_patt;
struct spr_sprite_pattern_set egg_patt;
struct spr_sprite_def eggspr;

struct vdp_hw_sprite bee_hw;

struct spr_sprite_def bee[10];
struct spr_sprite_def rats[10];

/**
 * NOTE : any initialized global data must be constant.
 */ 
const byte control_patt[8] = {255,255,255,255,255,255,255,255};
const byte control_colors [1] = {6};

void main()
{
	unsigned int count = 0;
	byte i;

	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	spr_init(1, 0);

	/**
	 * Low level sprites using direct access to VRAM
	 */
	vdp_copy_to_vram(bee1, vdp_base_sppat_grp1, 16 * 8);
	bee_hw.x = 100;
	bee_hw.y = 100;
	bee_hw.pattern = 0;
	bee_hw.color = 15;
	vdp_set_hw_sprite(0, &bee_hw);

	do {
	} while (sys_get_key(8) & 1);

	/**
	 * Single layer sprites with animation in two directions
	 */
	SPR_DEFINE_PATTERN_SET(bee_patt, spr_size_big, 1, 2, 2, bee1);
	SPR_DEFINE_PATTERN_SET(rat_patt, spr_size_big, 1, 2, 2, rat);
	SPR_DEFINE_PATTERN_SET(egg_patt, spr_size_big, 2, 3, 4, eggerland);

	spr_valloc_pattern_set(&bee_patt);
	spr_valloc_pattern_set(&rat_patt);
	spr_valloc_pattern_set(&egg_patt);

	for (i = 0; i< 10; i++) {
		SPR_DEFINE_SPRITE(bee[i], &bee_patt, 10, bee1_color);
		SPR_DEFINE_SPRITE(rats[i], &rat_patt, 10, rat_color);
		// set in random initial positions
		spr_set_pos(&bee[i], i * 20, i * 20);
		spr_set_pos(&rats[i], 16 + i * 20, 16 + i * 20);
		spr_show(&bee[i]);
		spr_show(&rats[i]);
	}

	SPR_DEFINE_SPRITE(eggspr, &egg_patt, 10, eggerland_color);
	spr_set_pos(&eggspr, 100, 100);
	spr_show(&eggspr);

	do {
		do {
			// delay a few ms
		} while (count++ < 0x01ff);
		count=0;
		for (i = 0; i< 10; i++) {
			spr_animate(&bee[i],1,-1,0);
			spr_animate(&rats[i],-1,1,0);
		}
	} while (1);

}

