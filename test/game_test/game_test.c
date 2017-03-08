/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "displ.h"
#include "phys.h"
#include "list.h"

#include "gen/game_test_tiles_ext.h"
#include "gen/game_test_sprites_ext.h"
#include "gen/game_test_maps.h"

#include <stdlib.h>

struct tile_set logo;
struct tile_set tileset_map1;
struct tile_set tileset_map2;
struct tile_set tileset_map3;
struct tile_set tileset_map4;
struct tile_set tileset_map5;

enum tile_sets_t {
	TILE_SCROLL,
	TILE_CHECKPOINT,
	TILE_CROSS,
	TILE_HEART,
	TILE_BELL,
	TILE_MAX,
};

struct tile_set tileset[TILE_MAX];
struct tile_object tileobject[31];

enum spr_patterns_t {
	PATRN_BAT,
	PATRN_RAT,
	PATRN_SPIDER,
	PATRN_TEMPLAR,
	PATRN_MONK,
	PATRN_MAX,
};

struct spr_pattern_set spr_pattern[PATRN_MAX];
struct spr_sprite_def enemy_sprites[31];
struct spr_sprite_def monk_sprite;

struct displ_object display_object[32];
struct displ_object dpo_arrow;
struct displ_object dpo_bullet[2];
struct displ_object dpo_monk;

struct list_head display_list;
struct list_head *elem, *elem2, *elem3;

struct animator animators[7];

enum anim_t {
	ANIM_LEFT_RIGHT,
	ANIM_GRAVITY,
	ANIM_STATIC,
	ANIM_JOYSTICK,
	ANIM_JUMP,
	ANIM_CYCLE_TILE,
};

struct map_object_item *map_object;

struct animator *anim;
struct displ_object *dpo;

uint8_t stick;
uint8_t scr_tile_buffer[768];

struct game_state_t {
	uint8_t map_x;	// position on the map in tile coordinates
	uint8_t map_y;
	uint8_t cross_cnt;
	uint8_t live_cnt;
} game_state;


void init_resources();

// void anim_up_down(struct displ_object *obj);
// void anim_drop(struct displ_object *obj);
void anim_static(struct displ_object *obj);
void anim_gravity(struct displ_object *obj);
void anim_left_right(struct displ_object *obj);
// void anim_horizontal_projectile(struct displ_object *obj);
void anim_joystick(struct displ_object *obj);
void anim_jump(struct displ_object *obj);
void anim_cycle_tile(struct displ_object *obj);

void animate_all();
// void spr_colision_handler();
void init_monk();
void init_game_state();
void init_animators();
void load_room();
void free_patterns();
void check_and_change_room();
void show_score_panel();
void find_room_data(struct map_object_item *map_obj);
void add_tileobject(struct displ_object *dpo, uint8_t objidx, enum tile_sets_t tileidx);
void add_sprite(struct displ_object *dpo, uint8_t objidx, enum spr_patterns_t pattidx);
void add_animator(struct displ_object *dpo, enum anim_t animidx);

void main()
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	//show_logo();
	//show_title_screen();

	sys_irq_init();
	phys_init();
	init_resources();
	init_game_state();
	init_animators();
	init_monk();
	load_room();
	show_score_panel();

	/** game loop **/
	for(;;) {
		stick = sys_get_stick(0);
		check_and_change_room();
		animate_all();
	}
}

void init_game_state()
{
	// room 3
	game_state.map_x = 96;
	game_state.map_y = 0;
}

void show_score_panel()
{
	// TODO: For this need to figure out how to solve the problem with the font
	//       the font will also be useful for other elements in the game.
	// best option is to use an MSX font instead of the original 16pixel one, either that or just
	// use the graphics and extend the size of the ROM
}


void check_and_change_room()
{
	bool change = false;
	if (dpo_monk.xpos > 240) {
		dpo_monk.xpos = 0;
		game_state.map_x+=32;
		change = true;
	} else if (dpo_monk.xpos == 0) {
		dpo_monk.xpos = 240;
		game_state.map_x-=32;
		change = true;
	}
	if (dpo_monk.ypos > 192 - 32) {
		dpo_monk.ypos = 0;
		game_state.map_y+=22;
		change = true;
	} else if (dpo_monk.ypos < -16) {
		dpo_monk.ypos = 192 - 32;
		game_state.map_y-=22;
		change = true;
	}
	if (change) {
		spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
		load_room();
	}
}

void load_room()
{
	uint8_t i, spr_ct = 0, tob_ct = 0;
	vdp_screen_disable();
	map_inflate_screen(map, scr_tile_buffer, game_state.map_x, game_state.map_y);

	spr_init();
	free_patterns();
	sys_set_rom();
	spr_valloc_pattern_set(&spr_pattern[PATRN_MONK]);
	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	sys_set_bios();

	INIT_LIST_HEAD(&display_list);
	find_room_data(map_object);
	for (dpo = display_object, i = 0; map_object->type != 255 ; i++, dpo++) {
		log_e("dpo %d type : %d\n", i ,map_object->type);
		if (map_object->type == ACTIONITEM) {
			if (map_object->object.actionitem.type == TYPE_SCROLL) {
				add_tileobject(dpo, tob_ct, TILE_SCROLL);
			} else if (map_object->object.actionitem.type == TYPE_TOGGLE) {
			} else if (map_object->object.actionitem.type == TYPE_CROSS) {
				add_tileobject(dpo, tob_ct, TILE_CROSS);
				add_animator(dpo, ANIM_CYCLE_TILE);
			} else if (map_object->object.actionitem.type == TYPE_TELETRANSPORT) {
			} else if (map_object->object.actionitem.type == TYPE_HEART) {
				add_tileobject(dpo, tob_ct, TILE_HEART);
				add_animator(dpo, ANIM_CYCLE_TILE);
			} else if (map_object->object.actionitem.type == TYPE_CHECKPOINT) {
				add_tileobject(dpo, tob_ct, TILE_CHECKPOINT);
			} else if (map_object->object.actionitem.type == TYPE_SWITCH) {
			} else if (map_object->object.actionitem.type == TYPE_CUP) {
			} else if (map_object->object.actionitem.type == TYPE_TRIGGER) {
			} else if (map_object->object.actionitem.type == TYPE_BELL) {
				add_tileobject(dpo, tob_ct, TILE_BELL);
			} else {
				map_object++;
				continue;
			}
			map_object++;
			tob_ct++;
		} else if (map_object->type == STATIC) {
			// TYPE_LAVA
			// TYPE_SPEAR
			// TYPE_DRAGON
			// TYPE_WATER
			map_object++;
		} else if (map_object->type == GHOST) {
			map_object++;
		} else if (map_object->type == ROPE) {
			map_object++;
		} else if (map_object->type == DOOR) {
			map_object++;
		} else if (map_object->type == SHOOTER) {
			// TYPE_FLUSH
			// TYPE_LEAK
			// TYPE_GARGOYLE
			// TYPE_ARCHER
			map_object++;
		} else if (map_object->type == BLOCK) {
			// TYPE_CROSS
			map_object++;
		} else if (map_object->type == STEP) {
			map_object++;
		} else if (map_object->type == MOVABLE) {
			if (map_object->object.movable.type == TYPE_TEMPLAR) {
				add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_BAT) {
				add_sprite(dpo, spr_ct, PATRN_BAT);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_SPIDER) {
				add_sprite(dpo, spr_ct, PATRN_SPIDER);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_RAT) {
				add_sprite(dpo, spr_ct, PATRN_RAT);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_WORM) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_PRIEST) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_FLY) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_SKELETON) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_PALADIN) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_DEATH) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_DARK_BAT) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_DEMON) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_SKELETON_CEIL) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_LAVA) {
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_SATAN) {
				map_object++;
				continue;
			} else {
				map_object++;
				continue;
			}
			map_object++;
			spr_ct++;
		} else {
			map_object++;
		}
	}
	INIT_LIST_HEAD(&dpo_monk.animator_list);
	list_add(&animators[ANIM_JOYSTICK].list, &dpo_monk.animator_list);
	list_add(&animators[ANIM_GRAVITY].list, &dpo_monk.animator_list);
	INIT_LIST_HEAD(&dpo_monk.list);
	list_add(&dpo_monk.list, &display_list);
	// show all elements
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			spr_show(dpo->spr);
		} else if (dpo->type == DISP_OBJECT_TILE) {
			log_e("showing dpo\n");
			tile_object_show(dpo->tob, scr_tile_buffer, false);
		}
	}
	vdp_copy_to_vram(scr_tile_buffer, vdp_base_names_grp1, 704);

	vdp_screen_enable();
}

void free_patterns()
{
	uint8_t i;

	for (i = 0; i < PATRN_MAX; i++)
		spr_pattern[i].allocated = false;
}


void add_tileobject(struct displ_object *dpo, uint8_t objidx, enum tile_sets_t tileidx)
{
	sys_set_rom();
	tile_set_valloc(&tileset[tileidx]);
	sys_set_bios();
	tileobject[objidx].x = map_object->x;
	tileobject[objidx].y = map_object->y;
	tileobject[objidx].cur_dir = 1;
	tileobject[objidx].cur_anim_step = 0;
	tileobject[objidx].ts = &tileset[tileidx];
	tileobject[objidx].idx = 0;
	dpo->type = DISP_OBJECT_TILE;
	dpo->tob = &tileobject[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
}

void add_sprite(struct displ_object *dpo, uint8_t objidx, enum spr_patterns_t pattidx)
{
	sys_set_rom();
	spr_valloc_pattern_set(&spr_pattern[pattidx]);
	spr_init_sprite(&enemy_sprites[objidx], &spr_pattern[pattidx]);
	sys_set_bios();
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_set_pos(&enemy_sprites[objidx], map_object->x, map_object->y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
}

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

void find_room_data(struct map_object_item *map_obj)
{
	uint8_t pos = game_state.map_x / 32 + (game_state.map_y / 22 * 5);
	map_object = (struct map_object_item *) object_index[pos];
	log_e("found room %d %d\n", pos, map_obj);
	log_e("should be room %d\n", room13);
}

void animate_all() {
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		phys_detect_tile_collisions(dpo, scr_tile_buffer);
		list_for_each(elem2, &dpo->animator_list) {
			anim = list_entry(elem2, struct animator, list);
			anim->run(dpo);
		}
	}
}

void init_monk()
{
	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	dpo_monk.xpos = 100;
	dpo_monk.ypos = 192 - 64;
	dpo_monk.type = DISP_OBJECT_SPRITE;
	dpo_monk.state = 0;
	dpo_monk.spr = &monk_sprite;
	spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
}

void anim_static(struct displ_object *obj)
{
	// do nothing, just make sure we can display the sprite
}

void anim_cycle_tile(struct displ_object *obj)
{
	//dpo->type = DISP_OBJECT_TILE;
	if (dpo->state++ == 10) {
		if (dpo->tob->cur_anim_step < dpo->tob->ts->n_frames) {
			tile_object_show(dpo->tob, scr_tile_buffer, true);
			dpo->tob->cur_anim_step++;
		} else {
			dpo->tob->cur_anim_step = 0;
		}
		dpo->state = 0;
	}
	// need to actually update VRAM for this tile... awesome.
}

void anim_jump(struct displ_object *obj)
{
	static uint8_t jmp_ct;

	if (obj->state == 1) {
		jmp_ct = 5;
		obj->state = 2;
	} else if (obj->state == 2){
		if (!is_colliding_up(obj)) {
			obj->ypos-=3;
			spr_animate(obj->spr, 0, -3 ,0);
		}
		if (--jmp_ct == 0 || is_colliding_up(obj)) {
			jmp_ct = 16;
			obj->state = 3;
		}
	} else if (obj->state == 3) {
		if (!is_colliding_up(obj)) {
			obj->ypos-=2;
			spr_animate(obj->spr, 0, -2 ,0);
		}
		if (--jmp_ct == 0 || is_colliding_up(obj)) {
			obj->state = 4;
		}
	} else if (obj->state == 4) {
		// wait for gravity to put us in the ground
		if (is_colliding_down(obj)) {
			list_del(&animators[ANIM_JUMP].list);
			obj->state = 0;
		}
	}
}

void anim_joystick(struct displ_object *obj)
{
	if (stick == STICK_LEFT || stick == STICK_UP_LEFT ||
		stick == STICK_DOWN_LEFT) {
		if (!is_colliding_left(obj)) {
			obj->xpos--;
			spr_animate(obj->spr, -1, 0 ,0);
		}
	}
	if (stick == STICK_RIGHT || stick == STICK_UP_RIGHT ||
		stick == STICK_DOWN_RIGHT) {
		if (!is_colliding_right(obj)) {
			obj->xpos++;
			spr_animate(obj->spr, 1, 0 ,0);
		}
	}
	if (stick == STICK_UP || stick == STICK_UP_RIGHT ||
		stick == STICK_UP_LEFT) {
		if (obj->state == 0 && is_colliding_down(obj)) {
			list_add(&animators[ANIM_JUMP].list, &dpo_monk.animator_list);
			obj->state = 1;
		}
	}
	if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT ||
		stick == STICK_DOWN_RIGHT) {
			// TODO: Duck
			// need change the monk sprite
			// and the dimensions to check collisions
	}
}

/*
 * Vertical fall at constant speed until collision
 */
void anim_gravity(struct displ_object *obj)
{
	if (!is_colliding_down(obj)) {
		obj->ypos++;
		spr_animate(obj->spr, 0, 1, 0);
	}
}

void anim_left_right(struct displ_object *obj)
{
	// FIXME: a-posteriory correction should never be necessary
	if (obj->state == 0 && !is_colliding_right(obj)) {
		obj->xpos++;
		spr_animate(obj->spr, 1, 0, 0);
	} else if (obj->state == 1 && !is_colliding_left(obj)) {
		obj->xpos--;
		spr_animate(obj->spr, -1, 0, 0);
	}
	if (is_colliding_left(obj)) {
		obj->state = 0;
	} else if (is_colliding_right(obj)) {
		obj->state = 1;
	}
	if (!is_colliding_down(obj)) {
		if (obj->state == 0) {
			obj->state = 1;
			obj->xpos-=2;
			spr_animate(obj->spr, -2, 0, 0);
		} else {
			obj->xpos+=2;
			spr_animate(obj->spr, 2, 0, 0);
			obj->state = 0;
		}
		//phys_detect_tile_collisions(obj, map_buf);
	}
}

void init_animators()
{
	animators[ANIM_LEFT_RIGHT].run = anim_left_right;
	animators[ANIM_GRAVITY].run = anim_gravity;
	animators[ANIM_STATIC].run = anim_static;
	// animators[3].run = anim_up_down;
	// animators[4].run = anim_drop;
	animators[ANIM_JOYSTICK].run = anim_joystick;
	animators[ANIM_JUMP].run = anim_jump;
	animators[ANIM_CYCLE_TILE].run = anim_cycle_tile;
}

void init_resources()
{
	uint8_t i;
	tile_init();

	sys_set_rom();
	/** initialize static tile sets for map data */
	INIT_TILE_SET(tileset_map1, maptiles1);
	INIT_TILE_SET(tileset_map2, maptiles2);
	INIT_TILE_SET(tileset_map3, maptiles3);
	INIT_TILE_SET(tileset_map4, maptiles4);
	INIT_TILE_SET(tileset_map5, maptiles5);

	/** allocate static tiles for map */
	tile_set_valloc(&tileset_map1);
	tile_set_valloc(&tileset_map2);
	tile_set_valloc(&tileset_map3);

	/** fixed index allocations for map consistency **/
	tile_set_to_vram(&tileset_map4, 126);
	tile_set_to_vram(&tileset_map5, 126 + 32);

	/** initialize dynamic tile sets */
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SCROLL], scroll, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CHECKPOINT], checkpoint, 2, 3, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CROSS], cross, 2, 2, 4, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_HEART], hearth, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_BELL], bell, 2, 2, 1, 2);

	/** initialize sprite pattern sets */
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_BAT], SPR_SIZE_16x16, 1, 1, 2, bat);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_RAT], SPR_SIZE_16x16, 1, 2, 2, rat);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SPIDER], SPR_SIZE_16x16, 1, 1, 2, spider);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_MONK], SPR_SIZE_16x32, 1, 2, 3, monk1);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_TEMPLAR], SPR_SIZE_16x32, 1, 2, 2, templar);
	sys_set_bios();

	// FIXME: this needs to be done per room
	for (i = 1; i < 76; i++)
		phys_set_colliding_tile(i);

}
