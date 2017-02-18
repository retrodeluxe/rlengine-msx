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
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "gen/phys_test.h"
#include <stdlib.h>
#include "displ.h"

struct spr_sprite_pattern_set pattern_smiley;
struct spr_sprite_pattern_set pattern_bullet;
struct spr_sprite_def enemy_sprites[32];
struct display_object display_list[32];
struct animator simple_anim;
struct tile_set tileset_kv;
struct map_object_item *map_object;

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

  // this should be a macro... map_inflate(tilemap, dest_buffer);
  map_inflate(tilemap_cmpr_dict, tilemap, map_buf, tilemap_cmpr_size, tilemap_w);
  vdp_copy_to_vram(map_buf, vdp_base_names_grp1, 768);

	SPR_DEFINE_PATTERN_SET(pattern_smiley, SPR_SIZE_16x16, 1, 2, 2, smiley);
  spr_valloc_pattern_set(&pattern_smiley);
	SPR_DEFINE_PATTERN_SET(pattern_bullet, SPR_SIZE_16x16, 1, 2, 2, bullet);
  spr_valloc_pattern_set(&pattern_bullet);

  //simple_anim.state = 0;
  simple_anim.next = simple_translation;

  map_object = (struct map_object_item *) objects;
	for (i = 0; i < objects_nitems; i++) {
		if (map_object->type == SPRITE) {
			if (map_object->object.sprite.type == TYPE_SMILEY) {
				SPR_DEFINE_SPRITE(enemy_sprites[i], &pattern_smiley, 6, smiley_color);
			} else {
				// we only consider smileys at the moment
			}
			spr_set_pos(&enemy_sprites[i], map_object->x, map_object->y);
			display_list[i].type = DISPLAY_OBJECT_SPRITE;
    	display_list[i].spr = &enemy_sprites[i];
			display_list[i].animator = &simple_anim;
			map_object++;
		}
	}

	for (i = 0; i < objects_nitems; i++ ) {
    // show the items
		if (display_list[i].type == DISPLAY_OBJECT_SPRITE) {
					spr_show(display_list[i].spr);
			}
	}

	do {
		for (i = 0; i < objects_nitems; i++ ) {
			// this will update flags about current collision state in the display list
			// those flags can be used by the animators
			//phys_detect_collisions(display_list);
			display_list[i].animator->next(&display_list[i]);
			// physics?
		}
	} while (sys_get_key(8) & 1);

}

// spr_animate actually -- becomes strange here because we need
// to handle as well special states of every sprite
// (e.g. dead, explosions, etc)
// need to collect uses cases I guess to find what is needed

void simple_translation(struct display_object *obj)
{
	obj->state++;
	if (obj->state < 40) {
		spr_animate(obj->spr, 1, 0, 0);
	} else if (obj->state >= 40) {
		spr_animate(obj->spr, -1, 0, 0);
	}
	if (obj->state > 80) {
		obj->state = 0;
	}
}

void gravity(struct display_object *obj)
{
	if (obj->collision_state != 5) {
		spr_animate(obj->spr, 0, 1, 0);
	}
}
