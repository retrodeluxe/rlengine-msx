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

#include "gen/phys_test.h"

struct spr_pattern_set pattern_smiley;
struct spr_pattern_set pattern_bullet;
struct spr_pattern_set pattern_skeleton;
struct spr_pattern_set pattern_spider;
struct spr_pattern_set pattern_arrow;
struct spr_pattern_set pattern_plant;
struct spr_pattern_set pattern_waterdrop;

struct spr_sprite_def enemy_sprites[32];
struct displ_object display_object[32];
struct list_head display_list;
struct animator animators[5];
struct tile_set tileset_kv;
struct map_object_item *map_object;
struct list_head *elem, *elem2;
struct animator *anim;
struct displ_object *dpo;

uint8_t map_buf[768];

void anim_up_down(struct displ_object *obj);
void anim_drop(struct displ_object *obj);
void anim_gravity(struct displ_object *obj);
void anim_left_right(struct displ_object *obj);
void anim_horizontal_projectile(struct displ_object *obj);

void main()
{
	uint8_t i,d;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	spr_init(1,0);

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	// FIXME this should be a macro... map_inflate(tilemap, dest_buffer);
	map_inflate(tilemap_cmpr_dict, tilemap, map_buf, tilemap_cmpr_size, tilemap_w);
	vdp_copy_to_vram(map_buf, vdp_base_names_grp1, 768);

	SPR_DEFINE_PATTERN_SET(pattern_smiley, SPR_SIZE_16x16, 1, 2, 2, smiley);
	SPR_DEFINE_PATTERN_SET(pattern_bullet, SPR_SIZE_16x16, 1, 2, 2, bullet);
	SPR_DEFINE_PATTERN_SET(pattern_skeleton, SPR_SIZE_16x32, 1, 2, 2, archer_skeleton);
	SPR_DEFINE_PATTERN_SET(pattern_arrow, SPR_SIZE_16x16, 1, 2, 1, arrow);
	SPR_DEFINE_PATTERN_SET(pattern_plant, SPR_SIZE_16x16, 1, 1, 2, plant);
	SPR_DEFINE_PATTERN_SET(pattern_waterdrop, SPR_SIZE_16x16, 1, 1, 3, waterdrop);
	SPR_DEFINE_PATTERN_SET(pattern_spider, SPR_SIZE_16x16, 1, 1, 2, spider);

	spr_valloc_pattern_set(&pattern_smiley);
	//spr_valloc_pattern_set(&pattern_bullet);
	spr_valloc_pattern_set(&pattern_skeleton);
	spr_valloc_pattern_set(&pattern_arrow);
	spr_valloc_pattern_set(&pattern_plant);
	spr_valloc_pattern_set(&pattern_waterdrop);
	spr_valloc_pattern_set(&pattern_spider);

	animators[0].run = anim_left_right;
	animators[1].run = anim_gravity;
	animators[2].run = anim_horizontal_projectile;
	animators[3].run = anim_up_down;
	animators[4].run = anim_drop;

	INIT_LIST_HEAD(&display_list);
	map_object = (struct map_object_item *) objects;
	for (dpo = display_object, i = 0; i < objects_nitems; i++, dpo++) {
		if (map_object->type == SPRITE) {
			if (map_object->object.sprite.type == TYPE_SMILEY) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_smiley, 6, smiley_color);
				INIT_LIST_HEAD(&dpo->animator_list);
				list_add(&animators[0].list, &dpo->animator_list);
				//list_add(&gravity_anim.list, &simple_anim.list);
			} else if (map_object->object.sprite.type == TYPE_ARCHER_SKELETON) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_skeleton, 6, archer_skeleton_color);
				enemy_sprites[i].cur_dir = 1;
				INIT_LIST_HEAD(&dpo->animator_list);
			} else if (map_object->object.sprite.type == TYPE_PLANT) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_plant, 6, plant_color);
				INIT_LIST_HEAD(&dpo->animator_list);
			} else if (map_object->object.sprite.type == TYPE_WATERDROP) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_waterdrop, 32, waterdrop_color);
				INIT_LIST_HEAD(&dpo->animator_list);
				list_add(&animators[4].list, &dpo->animator_list);

				// custom animator in for down direction
			} else if (map_object->object.sprite.type == TYPE_SPIDER) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_spider, 6, spider_color);
				INIT_LIST_HEAD(&dpo->animator_list);
				list_add(&animators[3].list, &dpo->animator_list);


				// custom animator up down with collision
			} else {
				map_object++;
				continue;
			}

			spr_set_pos(&enemy_sprites[i], map_object->x, map_object->y);

			dpo->type = DISP_OBJECT_SPRITE;
			dpo->spr = &enemy_sprites[i];
			dpo->xpos = map_object->x;
			dpo->ypos = map_object->y;
			dpo->state = 0;
			list_add(&dpo->list, &display_list);
			map_object++;
		}
	}

	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			// it seems we are showing one that should not.
			spr_show(dpo->spr);
		}
	}

	phys_init();
	phys_set_colliding_tile(1);

	// I want to add a new display object that is an arrow, that is fired by the skeleton
	// and disapears when it colides with a wall. A problem with this is that
	// I need dedicated dpo, and a need a way to make the arrow disapear - then appear again
	// later maybe.
	SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_arrow, 1, arrow_color);
	enemy_sprites[i].cur_dir = 1;
	spr_set_pos(&enemy_sprites[i], 32, 32);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[i];
	dpo->xpos = 32;
	dpo->ypos = 32;
	dpo->state = 0;
	spr_show(dpo->spr);
	INIT_LIST_HEAD(&dpo->animator_list);
	list_add(&animators[2].list, &dpo->animator_list);
	list_add(&dpo->list, &display_list);

	do {
		list_for_each(elem, &display_list) {
			dpo = list_entry(elem, struct displ_object, list);
			phys_detect_tile_collisions(dpo, map_buf);
			list_for_each(elem2, &dpo->animator_list) {
				anim = list_entry(elem2, struct animator, list);
				anim->run(dpo);
			}
		}
	} while (sys_get_key(8) & 1);
}



void anim_horizontal_projectile(struct displ_object *obj)
{
	//log_e("arrow animation\n");
	if (obj->state == 0 && !is_colliding_right(obj)) {
		obj->xpos+=3;
		spr_animate(obj->spr, 3, 0, 0);
	} else if (obj->state == 1 && !is_colliding_left(obj)) {
		obj->xpos-=3;
		spr_animate(obj->spr, -3, 0, 0);
	}
	if (is_colliding_left(obj) || is_colliding_right(obj)) {
		// trigger a self_removal?
	}
}

/**
 *  Horizontal translation staying on solid ground without fall
 */
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
		phys_detect_tile_collisions(obj, map_buf);
	}
}


void anim_drop(struct displ_object *obj) {
	static uint8_t start_y;
	static uint8_t frame_ct;

	if (obj->state == 0) {
		start_y = obj->ypos;
		frame_ct = 0;
		obj->state = 1;
	}
	// state 1 - just count
	if (obj->state == 1) {
		obj->spr->cur_anim_step = 0;
		if (frame_ct++ > 30) {
			frame_ct = 0;
			obj->state = 2;
		}
	}
	if (obj->state == 2) {
		obj->spr->cur_anim_step = 1;
		if (frame_ct++ > 30) {
			frame_ct = 0;
			obj->state = 3;
		}
	}
	if (obj->state == 3 && !is_colliding_down(obj)) {
		obj->ypos++;
		obj->spr->cur_anim_step = 2;
		spr_set_pos(obj->spr, obj->xpos, obj->ypos);
	}
	if (is_colliding_down(obj)) {
		obj->state = 0;
		obj->ypos = start_y;
		obj->spr->cur_anim_step = 0;
		spr_set_pos(obj->spr,obj->xpos, obj->ypos);
	}
	spr_update(obj->spr);
}
/*
 * Two state horizontal translation with direction switch on collision
 */
void anim_up_down(struct displ_object *obj)
{
	if (obj->state == 0 && !is_colliding_up(obj)) {
		obj->ypos--;
		spr_animate(obj->spr, 0, -1, 0);
	} else if (obj->state == 1 && !is_colliding_down(obj)) {
		obj->ypos++;
		spr_animate(obj->spr, 0, 1, 0);
	}
	if (is_colliding_down(obj)) {
		obj->state = 0;
	} else if (is_colliding_up(obj)) {
		obj->state = 1;
	}
}

/*
 * Vertical fall at constant speed until collision
 */
void anim_gravity(struct displ_object *obj)
{
	//log_e("gravity\n");
	if (!is_colliding_down(obj)) {
		obj->ypos++;
		spr_animate(obj->spr, 0, 1, 0);
	}
}
