/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "gen/spr_test.h"
#include <stdlib.h>

/**
 * Global data is placed in 0xC000 (RAM page 2) in 32K roms by default
 */

 enum spr_patterns_t {
 	PATRN_BEE,
 	PATRN_RAT,
 	PATRN_EGG,
 	PATRN_MONK,
 };

struct spr_sprite_def eggspr;
struct spr_sprite_def monk;

struct vdp_hw_sprite bee_hw;

struct spr_sprite_def bee[10];
struct spr_sprite_def rats[10];

/**
 * NOTE : any initialized global data must be constant.
 */
const uint8_t control_patt[8] = {255,255,255,255,255,255,255,255};
const uint8_t control_colors [1] = {6};

void main()
{
	unsigned int count = 0;
	uint8_t i;
	uint8_t two_states[] = {2,2};
	uint8_t four_states[] = {4,4,4};
	uint8_t monk_states[] = {3,3};

	vdp_set_mode(vdp_grp1);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	spr_init();
	/**
	 * Composite sprites
	 */
	SPR_DEFINE_PATTERN_SET(PATRN_MONK, SPR_SIZE_16x32, 1, 2, monk_states, monk1);
	spr_valloc_pattern_set(PATRN_MONK);

	// for (int i = 0; i< 32; i++) {
	// 	bee_hw.y = 0 + i * 8;
	// 	bee_hw.x = 100;
	// 	bee_hw.pattern = i * 4;
	// 	bee_hw.color = 10;
	// 	vdp_set_hw_sprite(&bee_hw, i);
	// }
	//
	// do {
	// } while (sys_get_key(8) & 1);

	spr_init_sprite(&monk, PATRN_MONK);
	spr_set_pos(&monk, 100, 100);
	spr_show(&monk);

	do {
		do {
			// delay a few ms
		} while (count++ < 0x01ff);
		count=0;
		spr_animate(&monk,-1,0);
	} while (sys_get_key(8) & 1);

	spr_init();

	/**
	 * Low level sprites using direct access to VRAM
	 */
	vdp_copy_to_vram(bee1, vdp_base_sppat_grp1, 16 * 8);
	bee_hw.x = 100;
	bee_hw.y = 100;
	bee_hw.pattern = 0;
	bee_hw.color = 15;
	vdp_set_hw_sprite(&bee_hw, 0);

	do {
	} while (sys_get_key(8) & 1);

	/**
	 * Single layer sprites with animation in two directions
	 */

	SPR_DEFINE_PATTERN_SET(PATRN_BEE, SPR_SIZE_16x16, 1, 2, two_states, bee1);
	SPR_DEFINE_PATTERN_SET(PATRN_RAT, SPR_SIZE_16x16, 1, 2, two_states, rat);
	SPR_DEFINE_PATTERN_SET(PATRN_EGG, SPR_SIZE_16x16, 2, 3, four_states, eggerland);

	spr_valloc_pattern_set(PATRN_BEE);
	spr_valloc_pattern_set(PATRN_RAT);
	spr_valloc_pattern_set(PATRN_EGG);

	for (i = 0; i< 10; i++) {
		spr_init_sprite(&bee[i], PATRN_BEE);
		spr_init_sprite(&rats[i], PATRN_RAT);
		// set in random initial positions
		spr_set_pos(&bee[i], i * 20, i * 20);
		spr_set_pos(&rats[i], 16 + i * 20, 16 + i * 20);
		spr_show(&bee[i]);
		spr_show(&rats[i]);
	}

	spr_init_sprite(&eggspr, PATRN_EGG);
	spr_set_pos(&eggspr, 100, 100);
	spr_show(&eggspr);

	do {
		do {
			// delay a few ms
		} while (count++ < 0x01ff);
		count=0;
		for (i = 0; i< 10; i++) {
			spr_animate(&bee[i],1,-1);
			spr_animate(&rats[i],-1,1);
		}
	} while (sys_get_key(8) & 1);


	do {
	} while (sys_get_key(8) & 1);
}
