/**
 *
 * Copyright (C) Retro DeLuxe 2022, All rights reserved.
 *
 */
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "dpo.h"
#include "phys.h"
#include "list.h"
#include "mem.h"
#include "ascii8.h"

#include <stdlib.h>

#include "gen/dpo_test_sprites_ext.h"
#include "gen/dpo_test_sprites_init.h"
#include "gen/dpo_test_tiles_ext.h"
#include "gen/map_defs.h"
#include "gen/map_init.h"

enum anim_t {
	ANIM_LEFT_RIGHT,
	ANIM_FALLING_BULLETS,
	ANIM_HORIZONTAL_PROJECTILE,
	ANIM_UP_DOWN,
	ANIM_DROP,
	ANIM_THROW_ARROW,
	ANIM_SPIT_BULLETS,
	ANIM_PLAYER,
	ANIM_MAX,
};

enum state_t {
	STATE_MOVING_LEFT,
	STATE_MOVING_RIGHT,
	STATE_MOVING_UP,
	STATE_MOVING_DOWN,
};

SpriteDef enemy_sprites[32];
SpriteDef arrow_sprite;
SpriteDef bullet_sprite[2];
SpriteDef monk_sprite;

DisplayObject *dpo;

TileSet tileset_kv;

uint8_t screen_buf[768];
uint16_t reftick;
uint8_t stick;

struct map_object_item *map_object;

void anim_up_down_bounded(DisplayObject *obj);
void anim_waterdrop(DisplayObject *obj);
void anim_falling_bullets(DisplayObject *obj);
void anim_left_right_bounded(DisplayObject *obj);
void anim_throw_arrow(DisplayObject *obj);
void anim_spit_bullets(DisplayObject *obj);
void anim_horizontal_projectile(DisplayObject *obj);
void anim_player(DisplayObject *obj);
void init_monk();

#pragma CODE_PAGE 3

/*
 * Initialize animators
 */
void init_animators()
{
	dpo_init_animators(ANIM_MAX);

	dpo_define_animator(ANIM_LEFT_RIGHT, anim_left_right_bounded);
	dpo_define_animator(ANIM_FALLING_BULLETS, anim_falling_bullets);
	dpo_define_animator(ANIM_HORIZONTAL_PROJECTILE, anim_horizontal_projectile);
	dpo_define_animator(ANIM_UP_DOWN, anim_up_down_bounded);
	dpo_define_animator(ANIM_DROP, anim_waterdrop);
	dpo_define_animator(ANIM_THROW_ARROW, anim_throw_arrow);
	dpo_define_animator(ANIM_SPIT_BULLETS, anim_spit_bullets);
	dpo_define_animator(ANIM_PLAYER, anim_player);
}

/*
 * Helper to add sprites to the display list
 */
static void add_sprite(DisplayObject *dpo, uint8_t objidx,
	enum spr_patterns_t pattidx, uint8_t x, uint8_t y)
{
	ascii8_set_data(4);
	spr_valloc_pattern_set(pattidx);
	spr_init_sprite(&enemy_sprites[objidx], pattidx);
	spr_set_pos(&enemy_sprites[objidx], x, y);
	spr_update(&enemy_sprites[objidx]);
	dpo->type = DPO_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = x;
	dpo->ypos = y;
	dpo->state = 0;
	dpo->collision_state = 0;
	dpo_display_list_add(dpo);
}

void main() __nonbanked
{
	uint8_t scene_idx, x, y;
	enum map_object_property_type spr_type;

	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

	mem_init();
	dpo_init();
	spr_init();
	spr_load_defs();

	ascii8_set_data(4);
	init_map_object_layers();

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	/* set tile map on screen */
	sys_memset(screen_buf,0,768);
	sys_memcpy(screen_buf, map_tilemap, 768);
	vdp_fastcopy_nametable(screen_buf);

	init_animators();
	
	/*
	 * Build the scene display list using map data
	 */
	map_object = (struct map_object_item *) map_object_layer[0];
	for (scene_idx = 0; map_object->type != 255; scene_idx++) {
        if (map_object->type == SPRITE) {
            spr_type = map_object->object.sprite.type;
            x = map_object->x; y = map_object->y;
			dpo = dpo_new();
            switch(spr_type) {
                case TYPE_SMILEY:
					dpo->max = map_object->object.sprite.max;
					dpo->min = map_object->object.sprite.min;
					dpo->speed = map_object->object.sprite.speed;
                    add_sprite(dpo, scene_idx, SPR_SMILEY, x, y);
                    dpo_add_animator(dpo, ANIM_LEFT_RIGHT);
                    break;
                case TYPE_ARCHER_SKELETON:
                    add_sprite(dpo, scene_idx, SPR_SKELETON, x, y);
                    dpo_add_animator(dpo, ANIM_THROW_ARROW);
                    enemy_sprites[scene_idx].state = 1;
                    enemy_sprites[scene_idx].frame = 1;
                    break;
                case TYPE_PLANT:
                    add_sprite(dpo, scene_idx, SPR_PLANT, x, y);
                    dpo_add_animator(dpo, ANIM_SPIT_BULLETS);
                    break;
                case TYPE_WATERDROP:
					dpo->max = map_object->object.sprite.max;
                    add_sprite(dpo, scene_idx, SPR_WATERDROP, x, y);
                    dpo_add_animator(dpo, ANIM_DROP);
                    break;
                case TYPE_SPIDER:
					dpo->max = map_object->object.sprite.max;
					dpo->min = map_object->object.sprite.min;
					dpo->speed = map_object->object.sprite.speed;
                    add_sprite(dpo, scene_idx, SPR_SPIDER, x, y);
                    dpo_add_animator(dpo, ANIM_UP_DOWN);
                    break;
                default:
                    continue;
            }
        }
		map_object++; /* we have a single type, so this works */
    }

	init_monk();
	dpo_show_all(screen_buf);
	sys_irq_init();

	/* game loop */
	for(;;) {
		sys_wait_vsync();
		reftick = sys_get_ticks();
		spr_refresh();
		stick = sys_get_stick(0);
		dpo_animate_all();

		/* fps limiter to 25/30
		 *   this ensures smooth animation if we stall at 50/60 fps 
		 *   when having too many objects on the screem.
		 */
		while (sys_get_ticks() - reftick < 1);
	}
}

void init_monk()
{
	spr_valloc_pattern_set(SPR_MONK);
	spr_init_sprite(&monk_sprite, SPR_MONK);
	dpo = dpo_new();
	dpo->xpos = 100;
	dpo->ypos = 192 - 48;
	dpo->type = DPO_SPRITE;
	dpo->state = 1;
	dpo->spr = &monk_sprite;
	dpo->collision_state = 0;
	spr_set_pos(&monk_sprite, dpo->xpos, dpo->ypos);
	dpo_display_list_add(dpo);
	dpo_add_animator(dpo, ANIM_PLAYER);
}

/**
 * Add arrow to the display list, including animation
 */
void throw_arrow(uint8_t xpos, uint8_t ypos)
{
	spr_valloc_pattern_set(SPR_ARROW);
	spr_init_sprite(&arrow_sprite, SPR_ARROW);	
	spr_set_pos(&arrow_sprite, xpos + 16, ypos + 16);
	arrow_sprite.state = 1;
	arrow_sprite.frame = 0;
	dpo = dpo_new();
	dpo->type = DPO_SPRITE;
	dpo->spr = &arrow_sprite;
	dpo->xpos = xpos + 16;
	dpo->ypos = ypos + 16;
	dpo->aux = 1;
	dpo->speed = 8;
	dpo->state = 0;
	dpo_display_list_add(dpo);
	dpo_add_animator(dpo, ANIM_HORIZONTAL_PROJECTILE);
	spr_show(dpo->spr);
}

/**
 * Add bullets to display list, including animations
 */
void throw_bullets(uint8_t xpos, uint8_t ypos)
{
	spr_valloc_pattern_set(SPR_BULLET);	
	spr_init_sprite(&bullet_sprite[0], SPR_BULLET);
	spr_init_sprite(&bullet_sprite[1], SPR_BULLET);
    for (int8_t i = 0; i < 2; i++) {
        bullet_sprite[i].frame = 0;
        spr_set_pos(&bullet_sprite[i], xpos + 8 * i, ypos - 8);
		dpo = dpo_new();
        dpo->type = DPO_SPRITE;
        dpo->spr = &bullet_sprite[i];
        dpo->xpos = xpos + 8 * i;
        dpo->ypos = ypos - 8;
        dpo->state = i;
		dpo->aux = 0;
		dpo->aux2 = i;
        dpo->collision_state = 0;
		dpo_display_list_add(dpo);
		dpo_add_animator(dpo, ANIM_FALLING_BULLETS);
		spr_show(dpo->spr);
    }
}

/*
 * Simple left-right player animation with stick input
 */
void anim_player(DisplayObject *obj)
{
	SpriteDef *sp = obj->spr;
	int8_t dx, dy;

	dx = 0;
	if (STICK_LEFT == stick)
		dx = -2;
	else if (STICK_RIGHT == stick)
		dx = 2;

 	if ((dx < 0 && obj->xpos > 16) || (dx > 0 && obj->xpos < 226))
		obj->xpos += dx;
	
	spr_animate(sp, dx, 0);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

/**
 * Animation for Skeleton to throw an arrow periodically
 */
void anim_throw_arrow(DisplayObject *obj)
{
    static uint16_t time_throw;
	static uint8_t frame_ct;

    if (obj->state == 0) {
        if (frame_ct++ > 60) {
            frame_ct = 0;
            obj->spr->frame = 0;
            spr_update(obj->spr);
            obj->state = 1;
            throw_arrow(obj->xpos, obj->ypos);
            time_throw = sys_gettime_secs();
        }
    } else {
        if (1 != obj->spr->frame && time_throw + 1 < sys_gettime_secs()) {
            obj->spr->frame = 1;
            spr_update(obj->spr);
        } else if (time_throw + 5 < sys_gettime_secs()) {
            obj->state = 0;
        }
    }
}
 

/**
 * Animation for plant to spit bullets periodically
 */
void anim_spit_bullets(DisplayObject *obj)
{
    static uint8_t frame_ct;
    static uint16_t time_throw;

    if (obj->state == 0) {
        if (frame_ct++ > 60) {
            frame_ct = 0;
            obj->spr->frame = 1;
            spr_update(obj->spr);
            obj->state = 1;
            throw_bullets(obj->xpos, obj->ypos);
            time_throw = sys_gettime_secs();
        }
    } else {
        if (0 != obj->spr->frame && time_throw + 1 < sys_gettime_secs()) {
            obj->spr->frame = 0;
            spr_update(obj->spr);
        } else if (time_throw + 2 < sys_gettime_secs()) {
            obj->state = 0;
        }
    }
}

/**
 * Horizontal projectile that disapears on wall
 */
void anim_horizontal_projectile(DisplayObject *obj)
{
  SpriteDef *sp = obj->spr;
  int8_t dx;

  if (obj->aux == 0)
    dx = -obj->speed;
  else
    dx = obj->speed;

  obj->xpos += dx;
  if (obj->xpos < 4 || obj->xpos > 235) {
    spr_hide(obj->spr);
    list_del(&obj->list);
	mem_free(obj);
  } else {
    spr_animate(sp, dx, 0);
    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
  }
}

/**
 * Simple left-right animation bounded by max/min coordinates
 */
void anim_left_right_bounded(DisplayObject *obj) 
{
  SpriteDef *sp = obj->spr;
  int8_t dx = 0;

  dx = obj->speed;
  switch (obj->state) {
	case STATE_MOVING_LEFT:
		if (obj->xpos > obj->max) {
			obj->state = STATE_MOVING_RIGHT;
			dx *= -1;
		}
		break;
	case STATE_MOVING_RIGHT:
		dx *= -1;
		if (obj->xpos < obj->min) {
			obj->state = STATE_MOVING_LEFT;
			dx *= -1;
		}
		break;
	default:
		obj->state = STATE_MOVING_LEFT;
		break;
  }

  obj->xpos += dx;
  spr_animate(sp, dx, 0);
  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);
}


/**
 * Animation of a water dropping from the ceiling
 */
void anim_waterdrop(DisplayObject *obj) {
  uint8_t max = obj->max;
  // here we assume all drops start at the same position
  if (obj->state == 0) {
    obj->aux2 = obj->ypos;
    obj->aux = 0;
    obj->state = 1;
  } else if (obj->state == 1) {
    obj->spr->frame = 0;
    if (obj->aux++ > 30) {
      obj->aux = 0;
      obj->state = 2;
    }
  } else if (obj->state == 2) {
    obj->spr->frame = 1;
    if (obj->aux++ > 30) {
      obj->aux = 0;
      obj->state = 3;
    }
  } else if (obj->state == 3 && obj->ypos < max) {
    obj->ypos += 4;
    obj->spr->frame = 2;
    spr_set_pos(obj->spr, obj->xpos, obj->ypos);
  } else if (obj->ypos >= max) {
    obj->aux = 0;
    obj->state = 0;
    obj->ypos = obj->aux2;
    obj->spr->frame = 0;
    spr_set_pos(obj->spr, obj->xpos, obj->ypos);
  }
  spr_update(obj->spr);
}

/**
 * Simple up-down animation bounded by max/min coordinates
 */
void anim_up_down_bounded(DisplayObject *obj) 
{
  SpriteDef *sp = obj->spr;
  int8_t dy = 0;

  dy = obj->speed;
  switch (obj->state) {
	case STATE_MOVING_DOWN:
		if (obj->ypos > obj->max) {
			obj->state = STATE_MOVING_UP;
			dy *= -1;
		}
		break;
	case STATE_MOVING_UP:
		dy *= -1;
		if (obj->ypos < obj->min) {
			obj->state = STATE_MOVING_DOWN;
			dy *= -1;
		}
		break;
	default:
		obj->state = STATE_MOVING_DOWN;
		break;
  }

  obj->ypos += dy;
  spr_animate(sp, 0, dy);
  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);
}

/**
 * Animation for bullets thrown up and to one side,
 *      following an inverted parabolic trajectory
 */
void anim_falling_bullets(DisplayObject *obj)
{
  SpriteDef *sp = obj->spr;
  int8_t dx = 0, dy = 0;

  if (obj->aux2 == 1) {
    dx = 2;
  } else if (obj->aux2 == 0) {
    dx = -2;
  }

  // inverted parabola with slope derivative = -1
  dy = obj->aux++;

  obj->xpos += dx;
  obj->ypos += dy;

  if (obj->ypos > 192 || obj->xpos > 250) {
    spr_hide(obj->spr);
    list_del(&obj->list);
	mem_free(obj);
  } else {
    spr_animate(sp, dx, dy);
    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
  }
}
