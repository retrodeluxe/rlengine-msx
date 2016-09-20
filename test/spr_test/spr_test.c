/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "vdp.h"
#include "sprite.h"
#include "gen/spr_test.h"

/**
 * Global data is placed in 0xC000 (RAM page 2) in 32K roms by default
 */
struct spr_sprite_def bee;
struct spr_sprite_def ratspr;
struct spr_sprite_def egg;
struct vdp_hw_sprite bee_hw;
	
/**
 * NOTE : any initialized global data must be constant.
 */ 
const byte control_patt[8] = {255,255,255,255,255,255,255,255};
const byte control_colors [1] = {6};



void main()
{
	unsigned int count = 0;
	byte dir = 1;

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
	DEFINE_HW_SPRITE(bee, SPR_SIZE_16x16, 1, 2, SPR_DIR_LR, 10, bee1, bee1_color);
	DEFINE_HW_SPRITE(ratspr, SPR_SIZE_16x16, 1, 2, SPR_DIR_LR, 10, rat, rat_color);
	DEFINE_HW_SPRITE(egg, SPR_SIZE_16x16, 2, 3, SPR_DIR_LRUP, 10, eggerland, eggerland_color);

	// the HW Sprite uses a pattern, that is allocated; if the pattern is already allocate,
	// there is no need to allocate it twice... so, how do we handle this?
	// 
	// DEFINE_SPRITE_PATTERN(spr_hw_pattern, size, planes, steps, directions, pattern)
	// DEFINE_SPRITE(spr, animation, pattern_handle, color) 

	// handle = spr_valloc_pattern(spr_pattern)
	// DEFINE_SPRITE(spr, anim, pattern, color)
	// spr_valloc_sprite(); --> this will reserve the attributes needed for this sprite in particular
	// spr_set_pos()
	// set_show
	// spr_move

	// this demo should actually be able to show up to 32 sprites on screen :) but only with the set of patterns we already have


	spr_valloc(&egg);	
	spr_valloc(&bee);
	spr_valloc(&ratspr);
	spr_set_pos(&bee, 0, 60);
	spr_set_pos(&ratspr, 0, 120);
	spr_set_pos(&egg, 100, 100);
	spr_show(&bee);
	spr_show(&ratspr);
	spr_show(&egg);


	dir = 0;
	do {
		do {
			// delay a few ms
		} while (count++ < 0x01ff);
		count=0;
		dir = sys_get_stick(0);
		spr_move(&bee,7,1,0);
		spr_move(&ratspr,3,1,0);
		if (dir != 0)
			spr_move(&egg,dir,1,0);
		
	} while (1);

}

