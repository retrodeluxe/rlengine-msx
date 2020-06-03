#pragma CODE_PAGE 5

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "dpo.h"
#include "phys.h"
#include "list.h"
#include "pt3.h"
#include "font.h"
#include "sfx.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"
#include "banks.h"

#include "gen/game_test_tiles_ext.h"
#include "gen/game_test_sprites_ext.h"
#include "gen/map_defs.h"

#include <stdlib.h>

extern struct tile_set tileset[TILE_MAX];
extern struct tile_set tileset_map[MAP_TILESET_MAX];

/** sprite definition data */
extern const uint8_t two_step_state[];
extern const uint8_t single_step_state[];
extern const uint8_t three_step_state[];
extern const uint8_t bullet_state[];
extern const uint8_t bat_state[];
extern const uint8_t waterdrop_state[];
extern const uint8_t single_four_state[];
extern const uint8_t spider_state[];
extern const uint8_t archer_state[];
extern const uint8_t jean_state[];

/** score panel primitives **/
struct tile_object score;
struct font big_digits;

void init_map_tilesets()
{
	tile_init();

	ascii8_set_data(PAGE_MAPTILES);
	// dragon + fire will take 55+
	// which means only the necessary static tiles should be loaded.

	INIT_TILE_SET(tileset_map[MAP_TILESET_1], maptiles1);
	INIT_TILE_SET(tileset_map[MAP_TILESET_2], maptiles2);
	INIT_TILE_SET(tileset_map[MAP_TILESET_3], maptiles3)
	INIT_TILE_SET(tileset_map[MAP_TILESET_4], maptiles4);
	INIT_TILE_SET(tileset_map[MAP_TILESET_5], maptiles5);

	tile_set_valloc(&tileset_map[MAP_TILESET_1]);
	tile_set_valloc(&tileset_map[MAP_TILESET_2]);
	tile_set_valloc(&tileset_map[MAP_TILESET_3]);
	tile_set_to_vram(&tileset_map[MAP_TILESET_4], MAP_TILESET_4_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_5], MAP_TILESET_5_POS);
}

void show_score_panel()
{
	uint8_t i;
	char snum[3];

	ascii8_set_data(PAGE_DYNTILES);
	tile_set_to_vram_bank(&tileset[TILE_HEART_STATUS], BANK2, 252 - 4);
	tile_set_to_vram_bank(&tileset[TILE_CROSS_STATUS], BANK2, 252 - 8);

	score.y = 192 - 16;
	score.x = 0;
	score.cur_dir = 1;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_HEART_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	score.x = 32;
	score.y = 192 - 16;
	score.cur_dir = 1;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_CROSS_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	_itoa(game_state.live_cnt, snum, 10);
	snum[2] = '~';

	font_vprint(&big_digits, 2, 22, snum);

	_itoa(game_state.cross_cnt, snum, 10);
	snum[2] = '~';

	font_vprint(&big_digits, 6, 22, snum);
}

void init_resources()
{
	init_map_tilesets();
	spr_init();

	/** initialize dynamic tile sets **/
	ascii8_set_data(PAGE_DYNTILES);

	INIT_DYNAMIC_TILE_SET(tileset[TILE_SCROLL], scroll, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CHECKPOINT], checkpoint, 2, 3, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CROSS], cross, 2, 2, 4, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_HEART], hearth, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_BELL], bell, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SWITCH], crosswitch, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TOGGLE], toggle, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TELETRANSPORT], portal, 2, 3, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CUP], cup, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_DRAGON], dragon, 11, 5, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_LAVA], lava, 1, 1, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SPEAR], spear, 1, 1, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_WATER], water, 1, 1, 8, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SATAN], satan, 4, 6, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_ARCHER_SKELETON], archer_skeleton, 2, 3, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_GARGOLYNE], gargolyne, 2, 2, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PLANT], plant, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PRIEST], priest, 2, 3, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_DOOR], door, 1, 4, 2, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TRAPDOOR], trapdoor, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_INVISIBLE_TRIGGER], invisible_trigger, 1, 4, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CROSS_STATUS], cross_status, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_HEART_STATUS], hearth_status, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_STAINED_GLASS], stainedglass, 6, 6, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SPLASH], splash, 2, 1, 3, 1);

	/** copy numeric font to vram **/
	ascii8_set_data(PAGE_INTRO);
	init_font(&big_digits, font_big_digits_tile, font_big_digits_tile_color, 10, 2,
		FONT_NUMERIC, 10, 1, 2);

	font_to_vram_bank(&big_digits, BANK2, 224);
}

void define_sprite(uint8_t pattidx)
{
	uint16_t size;
	uint8_t frames;

	ascii8_set_data(PAGE_SPRITES);
	switch(pattidx) {
		case PATRN_BAT:
			spr_define_pattern_set(PATRN_BAT, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_BAT, bat, bat_color);
			break;
		case PATRN_RAT:
			spr_define_pattern_set(PATRN_RAT, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_RAT, rat, rat_color);
			break;
		case PATRN_SPIDER:
			spr_define_pattern_set(PATRN_SPIDER, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_SPIDER, spider, spider_color);
			break;
		case PATRN_JEAN:
			spr_define_pattern_set(PATRN_JEAN, SPR_SIZE_16x32, 1, 7,
				jean_state);
			spr_copy_pattern_set(PATRN_JEAN, monk1, monk1_color);
			break;
		case PATRN_TEMPLAR:
			spr_define_pattern_set(PATRN_TEMPLAR, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_TEMPLAR, templar, templar_color);
			break;
		case PATRN_WORM:
			spr_define_pattern_set(PATRN_WORM, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_WORM, worm, worm_color);
			break;
		case PATRN_SKELETON:
			spr_define_pattern_set(PATRN_SKELETON, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_SKELETON, skeleton, skeleton_color);
			break;
		case PATRN_PALADIN:
			spr_define_pattern_set(PATRN_PALADIN, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_PALADIN, paladin, paladin_color);
			break;
		case PATRN_GUADANYA:
			spr_define_pattern_set(PATRN_GUADANYA, SPR_SIZE_16x16, 1, 1,
				single_four_state);
			spr_copy_pattern_set(PATRN_GUADANYA, guadanya, guadanya_color);
			break;
		case PATRN_GHOST:
			spr_define_pattern_set(PATRN_GHOST, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_GHOST, ghost, ghost_color);
			break;
		case PATRN_DEMON:
			spr_define_pattern_set(PATRN_DEMON, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_DEMON, demon, demon_color);
			break;
		case PATRN_DARKBAT:
			spr_define_pattern_set(PATRN_DARKBAT, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_DARKBAT, darkbat, darkbat_color);
			break;
		case PATRN_FLY:
			spr_define_pattern_set(PATRN_FLY, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_FLY, fly, fly_color);
			break;
		case PATRN_SKELETON_CEILING:
			spr_define_pattern_set(PATRN_SKELETON_CEILING, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_SKELETON_CEILING, skeleton_ceiling, skeleton_ceiling_color);
			break;
		case PATRN_FISH:
			spr_define_pattern_set(PATRN_FISH, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_FISH, fish, fish_color);
			break;
		case PATRN_FIREBALL:
			spr_define_pattern_set(PATRN_FIREBALL, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_FIREBALL, fireball, fireball_color);
			break;
		case PATRN_WATERDROP:
			spr_define_pattern_set(PATRN_WATERDROP, SPR_SIZE_16x16, 1, 1,
				waterdrop_state);
			spr_copy_pattern_set(PATRN_WATERDROP, waterdrop, waterdrop_color);
			break;
		case PATRN_BULLET:
			spr_define_pattern_set(PATRN_BULLET, SPR_SIZE_16x16, 1, 4,
				bullet_state);
			spr_copy_pattern_set(PATRN_BULLET, bullet, bullet_color);
			break;
	}

}
