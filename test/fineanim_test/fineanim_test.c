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
#include <stdlib.h>
#include "displ.h"
#include "phys.h"
#include "list.h"

#include "gen/fineanim_test.h"

/**
 *  test of physics collision and animation fine tuning.
 */

struct spr_pattern_set pattern_monk;

struct spr_sprite_def monk_sprite;
struct displ_object display_object[32];
struct displ_object dpo_arrow;
struct displ_object dpo_bullet[2];
struct displ_object dpo_monk;
struct list_head display_list;

enum anim_t {
	ANIM_LEFT_RIGHT,
	ANIM_GRAVITY,
	ANIM_STATIC,
	ANIM_JOYSTICK,
	ANIM_JUMP,
	ANIM_CYCLE_TILE,
};

enum obj_state_t {
	STATE_IDLE,
	STATE_JUMPING,
	STATE_ONAIR,
	STATE_ONCEILING,
	STATE_FALLING,
	STATE_LANDING,
	STATE_ONGROUND,
};

struct animator animators[7];
struct tile_set tileset_kv;
struct map_object_item *map_object;
struct list_head *elem, *elem2, *elem3;
struct animator *anim;
struct displ_object *dpo;


uint8_t map_buf[768];
uint8_t stick;

uint16_t ticks;

void anim_up_down(struct displ_object *obj);
void anim_drop(struct displ_object *obj);
void anim_gravity(struct displ_object *obj);
void anim_left_right(struct displ_object *obj);
void anim_horizontal_projectile(struct displ_object *obj);
void anim_joystick(struct displ_object *obj);
void anim_jump(struct displ_object *obj);
void animate_all();
void spr_colision_handler();
void init_monk();

void main()
{
	uint8_t i,d;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	spr_init();

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	map_inflate(tilemap_cmpr_dict, tilemap, map_buf, tilemap_cmpr_size, tilemap_w);

	vdp_copy_to_vram(map_buf, vdp_base_names_grp1, 768);

	SPR_DEFINE_PATTERN_SET(pattern_monk, SPR_SIZE_16x32, 1, 2, 3, monk1);
	spr_valloc_pattern_set(&pattern_monk);

	animators[ANIM_GRAVITY].run = anim_gravity;
	animators[ANIM_JOYSTICK].run = anim_joystick;
	animators[ANIM_JUMP].run = anim_jump;

	/* build scene */
	INIT_LIST_HEAD(&display_list);
	SPR_DEFINE_SPRITE(monk_sprite, &pattern_monk, 10, monk1_color);
	dpo_monk.xpos = 100;
	dpo_monk.ypos = 192 - 48;
	dpo_monk.type = DISP_OBJECT_SPRITE;
	dpo_monk.state = STATE_IDLE;
	dpo_monk.spr = &monk_sprite;
	spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
	INIT_LIST_HEAD(&dpo_monk.animator_list);
	list_add(&animators[ANIM_JOYSTICK].list, &dpo_monk.animator_list); // joystick
	list_add(&animators[ANIM_GRAVITY].list, &dpo_monk.animator_list); // gravity
	INIT_LIST_HEAD(&dpo_monk.list);
	list_add(&dpo_monk.list, &display_list);

	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			spr_show(dpo->spr);
		}
	}

	sys_irq_init();
	phys_init();
	phys_set_colliding_tile(1);
	//phys_set_sprite_collision_handler(spr_colision_handler);

	/* game loop */
	do {
		//ticks = sys_get_ticks();
		sys_sleep(10);
		//vdp_copy_to_vram(map_buf, vdp_base_names_grp1, 768);
		stick = sys_get_stick(0);
		animate_all();
		// do {
		// } while (sys_get_ticks() - ticks < 3);
	} while (1);
}

void animate_all() {
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		//phys_detect_tile_collisions(dpo, map_buf);
		list_for_each(elem2, &dpo->animator_list) {
			anim = list_entry(elem2, struct animator, list);
			anim->run(dpo);
		}
	}
}


void dpo_animate(struct displ_object *dpo, int8_t dx, int8_t dy)
{
	int8_t cdx, cdy;
	int16_t xpos, ypos;
	cdx = dx;
	cdy = dy;

	if (dx != 0 || dy != 0) {
		dpo->xpos += dx;
		dpo->ypos += dy;
		//phys_detect_tile_collisions(dpo, map_buf);
		if (dy > 0 && is_colliding_down(dpo)) {
			ypos = (dpo->ypos / 8) * 8;
			cdy = ypos - dpo->ypos + dy;
			dpo->ypos = ypos;
			if (dpo->state == STATE_FALLING)
				dpo->state = STATE_LANDING;
			else
				dpo->state = STATE_ONGROUND;
		}
		if (dpo->state == STATE_ONAIR &&
			is_colliding_up(dpo)) {
			ypos = (dpo->ypos / 8) * 8 + 6;
		 	cdy = ypos - dpo->ypos + dy;
		 	dpo->ypos = ypos;
			dpo->state = STATE_ONCEILING;
			log_e("ceiling\n");
		}
		if (is_colliding_left(dpo) || is_colliding_right(dpo)) {
			dpo->xpos -= dx;
			cdx = 0;
		}
		if (cdx != 0 || cdy != 0)
			spr_animate(dpo->spr, cdx, cdy, 0);
	}
}

void anim_jump(struct displ_object *obj)
{
	static uint8_t jmp_ct;

	if (obj->state == STATE_JUMPING) {
		obj->state = STATE_ONAIR;
		obj->vy = - 7;
		jmp_ct = 5;
	} else if (obj->state == STATE_ONAIR) {
		dpo_animate(obj, 0, obj->vy);
		if (obj->state == STATE_ONAIR) {
			obj->vy += 1;
			if (obj->vy > 0)
				obj->state = STATE_ONCEILING;
		}
	} else if (obj->state == STATE_ONCEILING) {
		obj->vy = 0;
		dpo_animate(obj, 0, obj->vy);
		if (--jmp_ct == 0) {
			obj->state = STATE_FALLING;
		}
	} else if (obj->state == STATE_FALLING) {
		// animate by gravity
	} else if (obj->state == STATE_LANDING) {
		list_del(&animators[ANIM_JUMP].list);
		obj->state = STATE_ONGROUND;
	}
}

void anim_joystick(struct displ_object *obj)
{
	if (stick == STICK_LEFT || stick == STICK_UP_LEFT ||
		stick == STICK_DOWN_LEFT) {
			dpo_animate(obj, -1, 0);
	}
	if (stick == STICK_RIGHT || stick == STICK_UP_RIGHT ||
		stick == STICK_DOWN_RIGHT) {
			dpo_animate(obj, 1, 0);
	}
	if (stick == STICK_UP || stick == STICK_UP_RIGHT ||
		stick == STICK_UP_LEFT) {
		if (obj->state == STATE_ONGROUND) {
			dpo_animate(obj, 0, -1);
			list_add(&animators[ANIM_JUMP].list, &dpo_monk.animator_list);
			obj->state = STATE_JUMPING;
		}
	}
	if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT ||
		stick == STICK_DOWN_RIGHT) {
	}
	//log_e("state : %d\n", dpo->state);
}

void anim_gravity(struct displ_object *obj)
{
	if (obj->state != STATE_JUMPING &&
	  	obj->state != STATE_ONAIR &&
		obj->state != STATE_ONCEILING) {
		dpo_animate(obj, 0, obj->vy);
		if(obj->vy++ > 2)
			obj->vy = 2;
	}
	//log_e("gravity here vy = %d\n", obj->vy);
}
