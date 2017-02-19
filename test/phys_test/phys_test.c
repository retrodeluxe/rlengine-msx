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

struct spr_sprite_pattern_set pattern_smiley;
struct spr_sprite_pattern_set pattern_bullet;
struct spr_sprite_def enemy_sprites[32];
struct display_object display_list[32];
struct animator simple_anim;
struct animator gravity_anim;
struct tile_set tileset_kv;
struct map_object_item *map_object;
struct list_head *ptr;
struct animator *anim;

uint8_t map_buf[768];

void simple_translation(struct display_object *obj);
void gravity(struct display_object *obj);

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
	spr_valloc_pattern_set(&pattern_smiley);
	SPR_DEFINE_PATTERN_SET(pattern_bullet, SPR_SIZE_16x16, 1, 2, 2, bullet);
	spr_valloc_pattern_set(&pattern_bullet);

	simple_anim.run = simple_translation;
	gravity_anim.run = gravity;

	map_object = (struct map_object_item *) objects;
	for (i = 0; i < objects_nitems; i++) {
		if (map_object->type == SPRITE) {
			if (map_object->object.sprite.type == TYPE_SMILEY) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_smiley, 6, smiley_color);
			} else {
				// we only consider smileys at the moment
			}
			spr_set_pos(&enemy_sprites[i], map_object->x, map_object->y);
			display_list[i].type = DISP_OBJECT_SPRITE;
			display_list[i].spr = &enemy_sprites[i];
			INIT_LIST_HEAD(&display_list[i].animator_list);
			list_add(&simple_anim.list, &display_list[i].animator_list);
			list_add(&gravity_anim.list, &simple_anim.list);
			display_list[i].xpos = map_object->x;
			display_list[i].ypos = map_object->y;
			display_list[i].state = 0;
			map_object++;
		}
	}

	for (i = 0; i < objects_nitems; i++ ) {
		if (display_list[i].type == DISP_OBJECT_SPRITE) {
			spr_show(display_list[i].spr);
		}
	}

	phys_init();
	phys_set_colliding_tile(1);

	do {
		for (i = 0; i < objects_nitems; i++ ) {
			phys_detect_tile_collisions(&display_list[i], map_buf);
			//display_list[i].animator->run(&display_list[i]);
			list_for_each(ptr, &(display_list[i].animator_list)) {
				//log_e("actually iterating %d\n", ptr);
				anim = list_entry(ptr, struct animator, list);
				anim->run(&display_list[i]);
			}
		}
	} while (sys_get_key(8) & 1);
}


/*
 * Two state horizontal translation with direction switch on collision
 */
void simple_translation(struct display_object *obj)
{
	//log_e("trans\n");
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
}

/*
 * Vertical fall at constant speed until collision
 */
void gravity(struct display_object *obj)
{
	//log_e("gravity\n");
	if (!is_colliding_down(obj)) {
		obj->ypos++;
		spr_animate(obj->spr, 0, 1, 0);
	}
}
