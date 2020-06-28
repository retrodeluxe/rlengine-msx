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
#include "gen/game_test_fonts_ext.h"
#include "gen/game_test_sprites_ext.h"
#include "gen/map_defs.h"

#include "gen/title_escape_ext.h"
#include "gen/title_death_is_close_ext.h"
#include "gen/title_abandoned_church_ext.h"
#include "gen/title_pestilent_beast_ext.h"
#include "gen/title_cave_of_illusions_ext.h"
#include "gen/title_hangman_tree_ext.h"
#include "gen/title_prayerofhope_ext.h"
#include "gen/title_underground_river_ext.h"
#include "gen/title_hidden_garden_ext.h"
#include "gen/title_satan_ext.h"
#include "gen/title_underground_river_ext.h"
#include "gen/title_unexpected_gate_ext.h"
#include "gen/title_lake_of_despair_ext.h"
#include "gen/title_ashes_to_ashes_ext.h"
#include "gen/title_the_altar_ext.h"
#include "gen/title_wheel_of_faith_ext.h"
#include "gen/title_banquet_of_death_ext.h"
#include "gen/title_evil_church_ext.h"
#include "gen/title_tortured_souls_ext.h"
#include "gen/title_wine_supplies_ext.h"
#include "gen/title_gloomy_tunnels_ext.h"
#include "gen/title_catacombs_ext.h"
#include "gen/title_plagued_ruins_ext.h"
#include "gen/title_tower_of_the_bell_ext.h"

#include <stdlib.h>

extern struct tile_set tileset[TILE_MAX];
extern struct tile_set tileset_map[MAP_TILESET_MAX];
extern struct tile_set tileset_room_title[ROOM_MAX];

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
extern const uint8_t spit_state[];
extern const uint8_t death_state[];

/** score panel primitives **/
struct tile_object score;
struct font big_digits;
struct font_set score_font_set;

void init_map_tilesets()
{
	tile_init();
	phys_init();

	ascii8_set_data(PAGE_MAPTILES);

	INIT_TILE_SET(tileset_map[MAP_TILESET_FOREST], map1);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CHURCH_1], map2);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CHURCH_2], map3);
	INIT_TILE_SET(tileset_map[MAP_TILESET_TREES_1], map4);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CAVE_1], map5);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CATACOMBS_1], map6);
	INIT_TILE_SET(tileset_map[MAP_TILESET_BEAM], map7);
	INIT_TILE_SET(tileset_map[MAP_TILESET_STONE], map8);
	INIT_TILE_SET(tileset_map[MAP_TILESET_FLAMES], map9);
	INIT_TILE_SET(tileset_map[MAP_TILESET_BRICKS], map10);
	INIT_TILE_SET(tileset_map[MAP_TILESET_BRICKS_2], map11);
	INIT_TILE_SET(tileset_map[MAP_TILESET_PLANT], map12);
	INIT_TILE_SET(tileset_map[MAP_TILESET_FOREST_2], map13);
	INIT_TILE_SET(tileset_map[MAP_TILESET_GRAVES], map14);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CHURCH_DECO], map15);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CHURCH_DECO_2], map16);
	INIT_TILE_SET(tileset_map[MAP_TILESET_X], map17);
	INIT_TILE_SET(tileset_map[MAP_TILESET_HANGING_PLANT], map18);
	INIT_TILE_SET(tileset_map[MAP_TILESET_SKULL], map19);
	INIT_TILE_SET(tileset_map[MAP_TILESET_DEATH_DECO], map20);
	INIT_TILE_SET(tileset_map[MAP_TILESET_SKULL_2], map21);
	INIT_TILE_SET(tileset_map[MAP_TILESET_ROPE], map22);
	INIT_TILE_SET(tileset_map[MAP_TILESET_CAVE_2], map23);
}

void clear_map_tilesets()
{
	uint8_t i;

	for (i = 0; i < MAP_TILESET_MAX; i++) {
		tile_set_vfree(&tileset_map[i]);
	}
}

static void init_tiles_zone_1()
{
	tile_set_to_vram(&tileset_map[MAP_TILESET_FOREST], MAP_TILESET_FOREST_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_TREES_1], MAP_TILESET_TREES_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_FOREST_2], MAP_TILESET_FOREST_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_PLANT], MAP_TILESET_PLANT_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CHURCH_DECO], MAP_TILESET_CHURCH_DECO_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_GRAVES], MAP_TILESET_GRAVES_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CHURCH_1], MAP_TILESET_CHURCH_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CHURCH_DECO_2], MAP_TILESET_CHURCH_DECO_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CHURCH_2], MAP_TILESET_CHURCH_2_POS);
}

static void init_tiles_zone_2()
{
	tile_set_to_vram(&tileset_map[MAP_TILESET_CATACOMBS_1], MAP_TILESET_CATACOMBS_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_DEATH_DECO], MAP_TILESET_DEATH_DECO_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS_2], MAP_TILESET_BRICKS_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CAVE_1], MAP_TILESET_CAVE_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_STONE], MAP_TILESET_STONE_POS);
}

static void init_tiles_zone_3()
{
	tile_set_to_vram(&tileset_map[MAP_TILESET_FOREST], MAP_TILESET_FOREST_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_HANGING_PLANT], MAP_TILESET_HANGING_PLANT_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CAVE_1], MAP_TILESET_CAVE_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_PLANT], MAP_TILESET_PLANT_POS);
}

static void init_tiles_zone_4()
{
	// XXX: Way to big, need to split in smaller chunks
	tile_set_to_vram(&tileset_map[MAP_TILESET_FOREST], MAP_TILESET_FOREST_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_HANGING_PLANT], MAP_TILESET_HANGING_PLANT_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CAVE_1], MAP_TILESET_CAVE_1_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_PLANT], MAP_TILESET_PLANT_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_SKULL_2], MAP_TILESET_SKULL_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_CHURCH_2], MAP_TILESET_CHURCH_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BEAM], MAP_TILESET_BEAM_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS_2], MAP_TILESET_BRICKS_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS], MAP_TILESET_BRICKS_POS);
}

static void init_tiles_zone_5()
{
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS], MAP_TILESET_BRICKS_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS_2], MAP_TILESET_BRICKS_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_SKULL_2], MAP_TILESET_SKULL_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_FLAMES], MAP_TILESET_FLAMES_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_DEATH_DECO], MAP_TILESET_DEATH_DECO_POS);
}

static void init_tiles_zone_6() {
	tile_set_to_vram(&tileset_map[MAP_TILESET_DEATH_DECO], MAP_TILESET_DEATH_DECO_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_SKULL_2], MAP_TILESET_SKULL_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_BRICKS_2], MAP_TILESET_BRICKS_2_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_STONE], MAP_TILESET_STONE_POS);
}

void init_room_tilesets(uint8_t room, bool reload)
{
	static uint8_t prev_room = ROOM_SATAN;

	phys_init();
	ascii8_set_data(PAGE_MAPTILES);

	// zone 1: forest and church
	if (room < ROOM_CAVE_DRAGON) {
		if (prev_room > ROOM_HAGMAN_TREE || reload) {
			clear_map_tilesets();
			init_tiles_zone_1();
		}
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_FOREST]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_TREES_1]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CHURCH_1]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CHURCH_2]);

		phys_clear_colliding_tile(16); // step brown
		phys_set_down_colliding_tile(16);

	/// zone 2: catacombs
	} else if (room == ROOM_CATACOMBS || room == ROOM_CATACOMBS_WHEEL
		|| room == ROOM_CATACOMBS_FLIES) {
		if (prev_room == ROOM_CAVE_GHOST || prev_room == ROOM_HIDDEN_GARDEN
			|| prev_room == ROOM_CAVE_LAKE || prev_room == ROOM_CHURCH_ALTAR
			|| reload) {
			clear_map_tilesets();
			init_tiles_zone_2();
		}
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CATACOMBS_1]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_BRICKS_2]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_STONE]);

		phys_clear_colliding_tile(38); // step white
		phys_set_down_colliding_tile(38);

	// zone 3: cave gardens
	} else if (room == ROOM_CAVE_DRAGON || room == ROOM_CAVE_GHOST
		|| room == ROOM_HIDDEN_RIVER || room == ROOM_HIDDEN_GARDEN) {
		if (prev_room == ROOM_CATACOMBS_FLIES
			|| prev_room == ROOM_CAVE_LAKE
			|| prev_room == ROOM_CATACOMBS
			|| prev_room == ROOM_CAVE_TUNNEL || reload) {
				clear_map_tilesets();
				init_tiles_zone_3();
			}
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_FOREST]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CAVE_1]);

		phys_clear_colliding_tile(38); // step white
		phys_set_down_colliding_tile(38);

	} else if (room == ROOM_DEATH) {
		init_tiles_zone_6();
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_BRICKS_2]);
	// zone 4: other cave rooms
	} else if (room == ROOM_CAVE_TUNNEL || room == ROOM_CAVE_LAKE
		|| room == ROOM_CAVE_GATE) {
		if (prev_room == ROOM_CAVE_DRAGON
			|| prev_room == ROOM_CATACOMBS_WHEEL
			|| prev_room == ROOM_CAVE_GHOST
			|| prev_room == ROOM_EVIL_CHURCH || reload) {
				clear_map_tilesets();
				init_tiles_zone_4();
			}
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CAVE_1]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_CHURCH_2]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_BRICKS]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_FOREST]);

		phys_clear_colliding_tile(38); // step white
		phys_set_down_colliding_tile(38);

	// zone 5: evil church
	} else if (room > ROOM_CAVE_GATE) {
		if (prev_room == ROOM_CAVE_GATE || reload) {
			clear_map_tilesets();
			init_tiles_zone_5();
		}
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_BRICKS]);
		phys_set_colliding_tile_set(&tileset_map[MAP_TILESET_BRICKS_2]);

	}

	prev_room = room;
}

void init_room_titles()
{
	ascii8_set_data(PAGE_ROOM_TITLES);
	INIT_TILE_SET(tileset_room_title[ROOM_PRAYER_OF_HOPE], title_prayerofhope);
	INIT_TILE_SET(tileset_room_title[ROOM_CHURCH_TOWER], title_tower_of_the_bell);
	INIT_TILE_SET(tileset_room_title[ROOM_CHURCH_WINE_SUPPLIES], title_wine_supplies);
	INIT_TILE_SET(tileset_room_title[ROOM_FOREST], title_escape);
	INIT_TILE_SET(tileset_room_title[ROOM_GRAVEYARD], title_death_is_close);
	INIT_TILE_SET(tileset_room_title[ROOM_CHURCH_ENTRANCE], title_abandoned_church);
	INIT_TILE_SET(tileset_room_title[ROOM_CHURCH_ALTAR], title_the_altar);
	INIT_TILE_SET(tileset_room_title[ROOM_HAGMAN_TREE], title_hangman_tree);
	INIT_TILE_SET(tileset_room_title[ROOM_CAVE_DRAGON], title_pestilent_beast);
	INIT_TILE_SET(tileset_room_title[ROOM_CAVE_GHOST], title_cave_of_illusions);
	INIT_TILE_SET(tileset_room_title[ROOM_CATACOMBS_FLIES], title_plagued_ruins);
	INIT_TILE_SET(tileset_room_title[ROOM_CATACOMBS], title_catacombs);
	INIT_TILE_SET(tileset_room_title[ROOM_HIDDEN_GARDEN], title_hidden_garden);
	INIT_TILE_SET(tileset_room_title[ROOM_CAVE_TUNNEL], title_gloomy_tunnels);
	INIT_TILE_SET(tileset_room_title[ROOM_CAVE_LAKE], title_lake_of_despair);
	INIT_TILE_SET(tileset_room_title[ROOM_CATACOMBS_WHEEL], title_wheel_of_faith);
	INIT_TILE_SET(tileset_room_title[ROOM_DEATH], title_banquet_of_death);
	INIT_TILE_SET(tileset_room_title[ROOM_HIDDEN_RIVER], title_underground_river);
	INIT_TILE_SET(tileset_room_title[ROOM_CAVE_GATE], title_unexpected_gate);
	INIT_TILE_SET(tileset_room_title[ROOM_EVIL_CHURCH], title_evil_church);
	INIT_TILE_SET(tileset_room_title[ROOM_EVIL_CHURCH_2], title_tortured_souls);
	INIT_TILE_SET(tileset_room_title[ROOM_EVIL_CHURCH_3], title_ashes_to_ashes);
	ascii8_set_data(PAGE_SPRITES);
	INIT_TILE_SET(tileset_room_title[ROOM_SATAN], title_satan);
}

void show_room_title(uint8_t room)
{
	uint8_t i, tile, w;
	uint16_t vram_offset;

	#define MAX_TITLE_LEN 18
	#define SCR_WIDTH 32

	if (room == ROOM_SATAN)
		ascii8_set_data(PAGE_SPRITES);
	else
		ascii8_set_data(PAGE_ROOM_TITLES);

	w = tileset_room_title[room].w;
	vram_offset = vdp_base_names_grp1 + SCR_WIDTH + 22 * SCR_WIDTH;

	tile_set_to_vram_bank(&tileset_room_title[room], BANK2, 180);

	vdp_memset(vram_offset - MAX_TITLE_LEN, MAX_TITLE_LEN, 0);
	vdp_memset(vram_offset - MAX_TITLE_LEN + SCR_WIDTH, MAX_TITLE_LEN, 0);

	tile = 180;
	for (i = 0; i < w; i++) {
		vdp_write(vram_offset - w + i, tile + i);
		vdp_write(vram_offset - w + i + SCR_WIDTH, tile + i + w);
	}
}

// FIXME: show score panel should operate on RAM and let load_screen update VRAM
//        _except_ for when we update counters of lives or crosses
void show_score_panel()
{
	uint8_t i;
	char snum[3];

	ascii8_set_data(PAGE_DYNTILES);
	tile_set_to_vram_bank(&tileset[TILE_HEART_STATUS], BANK2, 252 - 4);
	tile_set_to_vram_bank(&tileset[TILE_CROSS_STATUS], BANK2, 252 - 8);

	score.y = 192 - 16;
	score.x = 0;
	score.cur_dir = 0;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_HEART_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	score.x = 32;
	score.y = 192 - 16;
	score.cur_dir = 0;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_CROSS_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	vdp_write(vdp_base_names_grp1 + 3 + 22 * 32, 0);
	vdp_write(vdp_base_names_grp1 + 3 + 23 * 32, 0);

	score_font_set.numeric = &big_digits;
	_itoa(game_state.live_cnt, snum, 10);
	font_vprintf(&score_font_set, 2, 22, snum);
	_itoa(game_state.cross_cnt, snum, 10);
	font_vprintf(&score_font_set, 6, 22, snum);
}

void init_resources()
{
	init_map_tilesets();
	spr_init();

	/** initialize dynamic tile sets **/
	ascii8_set_data(PAGE_DYNTILES);

	INIT_DYNAMIC_TILE_SET(tileset[TILE_SCROLL], scroll, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_RED_SCROLL], red_scroll, 2, 2, 1, 1);
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
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SATAN], satan, 4, 7, 1, 5);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_ARCHER_SKELETON], archer_skeleton, 2, 3, 2, 2);
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
	INIT_DYNAMIC_TILE_SET(tileset[TILE_BLOCK_CROSS], cross_small, 1, 1, 4, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_INVERTED_CROSS], invertedcross, 2, 2, 3, 1);

	/** copy numeric font to vram **/
	ascii8_set_data(PAGE_MAPTILES);
	INIT_FONT(big_digits, font_big_digits, FONT_NUMERIC, 10, 1, 2);
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
		case PATRN_SCYTHE:
			spr_define_pattern_set(PATRN_SCYTHE, SPR_SIZE_16x16, 1, 1,
				single_four_state);
			spr_copy_pattern_set(PATRN_SCYTHE, scythe, scythe_color);
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
			spr_define_pattern_set(PATRN_DARKBAT, SPR_SIZE_32x16, 1, 2,
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
		case PATRN_ARROW:
			spr_define_pattern_set(PATRN_ARROW, SPR_SIZE_16x16, 1, 2,
				single_step_state);
			spr_copy_pattern_set(PATRN_ARROW, arrow, arrow_color);
			break;
		case PATRN_SPIT:
			spr_define_pattern_set(PATRN_SPIT, SPR_SIZE_16x16, 1, 1,
				spit_state);
			spr_copy_pattern_set(PATRN_SPIT, spit, spit_color);
			break;
		case PATRN_DEATH:
			spr_define_pattern_set(PATRN_DEATH, SPR_SIZE_32x32, 1, 2,
				death_state);
			spr_copy_pattern_set(PATRN_DEATH, death, death_color);
			break;
	}

}
