/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "msx.h"
#include "log.h"
#include "dpo.h"
#include "phys.h"
#include "sys.h"
#include "sprite.h"
#include "bitmap.h"
#include "tile.h"

#pragma CODE_PAGE 2

static uint8_t colliding_tiles[32];
static uint8_t colliding_tiles_down[32];
static uint8_t colliding_tiles_trigger[32];

static uint8_t tile[12];
static void (*sprite_colision_cb)();

static struct tile_collision_group cgroup[MAX_CROUPS];
static uint8_t n_cgroups;

#define STATFL 0xf3e7;
#define SPR_COLISION_MASK 32
#define SPR_5TH_ALIGN_MASK 64

void phys_check_collision_bit() __nonbanked
{
	uint8_t *status = (uint8_t *)STATFL;
	if ((*status & SPR_COLISION_MASK) != 0) {
		sprite_colision_cb();
	}
}

void phys_init()
{
	sys_memset(colliding_tiles, 255, 32);
	sys_memset(colliding_tiles_down, 255, 32);
	sys_memset(colliding_tiles_trigger, 255, 32);
	n_cgroups = 0;
}

/* note that this runs in interrupt context */
void phys_set_sprite_collision_handler(void (*handler)) __nonbanked
{
	sprite_colision_cb = handler;
	sys_irq_register(phys_check_collision_bit);
}

void phys_clear_sprite_collision_handler() __nonbanked
{
	// FIXME: unregister is hanging sometimes
	sys_irq_unregister(phys_check_collision_bit);
	sprite_colision_cb = NULL;
}

/**
 * set callbacks for specific tiles
 *
 *  FIXME: the same tile object repeated many times will create repeated cgroups
 */
void phys_set_tile_collision_handler(struct displ_object *dpo,
	void (*handler), uint8_t data)
{
	uint8_t base_tile = dpo->tob->ts->pidx;
	uint8_t num_tiles = dpo->tob->ts->frame_w * dpo->tob->ts->frame_h *
		dpo->tob->ts->n_frames * dpo->tob->ts->n_dirs;

	cgroup[n_cgroups].start = base_tile;
	cgroup[n_cgroups].end = base_tile + num_tiles - 1;
	cgroup[n_cgroups].handler = handler;
	cgroup[n_cgroups].data = data;
	cgroup[n_cgroups].dpo = dpo;
	n_cgroups++;
}

/**
 * Sets all the tiles composing an object as coliding tiles
 */
void phys_set_colliding_tile_object(struct displ_object *dpo,
	enum tile_collision_type type, void (*handler), uint8_t data)
{
	uint8_t i;
	uint8_t base_tile = dpo->tob->ts->pidx;
	uint8_t num_tiles = dpo->tob->ts->frame_w * dpo->tob->ts->frame_h *
		dpo->tob->ts->n_frames * dpo->tob->ts->n_dirs;

	for (i = base_tile; i < base_tile + num_tiles; i++) {
		if (type == TILE_COLLISION_DOWN)
			phys_set_down_colliding_tile(i);
		else if (type == TILE_COLLISION_TRIGGER)
			phys_set_trigger_colliding_tile(i);
		else
			phys_set_colliding_tile(i);
	}

	phys_set_tile_collision_handler(dpo, handler, data);
}

void phys_clear_colliding_tile_object(struct displ_object *dpo)
{
	uint8_t i;
	uint8_t base_tile = dpo->tob->ts->pidx;
	uint8_t num_tiles = dpo->tob->ts->frame_w * dpo->tob->ts->frame_h *
		dpo->tob->ts->n_frames * dpo->tob->ts->n_dirs;

	for (i = base_tile; i < base_tile + num_tiles; i++) {
		phys_clear_colliding_tile(i);
	}

	// TODO: clear the collision group as well
}


/*
 * if the tile has a handler set, notify
 * XXX: this is asking for optimization
 */
static void phys_tile_collision_notify(uint8_t tile) __nonbanked
{
	uint8_t i;
	for (i = 0; i < n_cgroups; i++) {
		if (tile >= cgroup[i].start && tile <= cgroup[i].end) {
			cgroup[i].handler(cgroup[i].dpo, cgroup[i].data);
		}
	}
}
/*
 * Set collision flag for a specific tile (all directions)
 */
void phys_set_colliding_tile(uint8_t tile)
{
	bitmap_reset(colliding_tiles, tile);
	bitmap_reset(colliding_tiles_down, tile);
	bitmap_reset(colliding_tiles_trigger, tile);
}

/**
 * Set a tile that collides only when falling
 */
void phys_set_down_colliding_tile(uint8_t tile)
{
	bitmap_reset(colliding_tiles_down, tile);
}

void phys_set_trigger_colliding_tile(uint8_t tile)
{
	bitmap_reset(colliding_tiles_trigger, tile);
}

void phys_clear_colliding_tile(uint8_t tile)
{
	bitmap_set(colliding_tiles, tile);
	bitmap_set(colliding_tiles_down, tile);
	bitmap_set(colliding_tiles_trigger, tile);
}

static bool is_coliding_tile_pair(uint8_t tile1, uint8_t tile2) __nonbanked
{
	return ((bitmap_get(colliding_tiles, tile1) == 0) ||
                 (bitmap_get(colliding_tiles, tile2) == 0));
}

static bool is_coliding_down_tile_pair(uint8_t tile1, uint8_t tile2) __nonbanked
{
	return ((bitmap_get(colliding_tiles_down, tile1) == 0) ||
                 (bitmap_get(colliding_tiles_down, tile2) == 0));
}

static bool is_coliding_tile_triplet(uint8_t tile1, uint8_t tile2, uint8_t tile3) __nonbanked
{
	return ((bitmap_get(colliding_tiles, tile1) == 0) ||
                 (bitmap_get(colliding_tiles, tile2) == 0) ||
                 (bitmap_get(colliding_tiles, tile3) == 0));
}

static bool is_coliding_trigger_tile_pair(uint8_t tile1, uint8_t tile2) __nonbanked
{
	return ((bitmap_get(colliding_tiles_trigger, tile1) == 0) ||
                 (bitmap_get(colliding_tiles_trigger, tile2) == 0));
}

#define TILE_WIDTH 32


/**
 * Update dpo collision_state
 */
static void phys_detect_tile_collisions_16x32(struct displ_object *obj,
        uint8_t *map, int8_t dx, int8_t dy, bool notify) __nonbanked
{
	uint8_t x, y, c;
	uint8_t *base_ceiling_l, *base_ceiling_r, *base_left_t, *base_left_b;
	uint8_t *base_right_t, *base_right_b, *base_floor_l, *base_floor_r;

	x = obj->xpos;
	y = obj->ypos;

	base_ceiling_l = map + x / 8 + (y + 7) / 8 * TILE_WIDTH;
	base_ceiling_r = map + (x + 8) / 8 + (y + 7) / 8 * TILE_WIDTH;

	base_left_t = map + (x - 1) / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_left_b = map + (x - 1) / 8 + (y + 24) / 8 * TILE_WIDTH;

	base_right_t = map + (x + 15) / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_right_b = map + (x + 15) / 8 + (y + 24) / 8 * TILE_WIDTH;

	// collision down does not depend on dy
	base_floor_l = map + x / 8 + (y + 32) / 8 * TILE_WIDTH;
	base_floor_r = map + (x + 8) / 8 + (y + 32) / 8 * TILE_WIDTH;

	tile[0] = *(base_ceiling_l);
	tile[1] = *(base_ceiling_r);
	tile[2] = *(base_left_t);
	tile[3] = *(base_left_b);
	tile[4] = *(base_right_t);
	tile[5] = *(base_right_b);
	tile[6] = *(base_floor_l);
	tile[7] = *(base_floor_r);

	obj->collision_state = 0;

	/** check non-blocking collisions **/
	if (notify) {
		if (is_coliding_trigger_tile_pair(tile[4], tile[5])) {
			phys_tile_collision_notify(tile[4]);
			phys_tile_collision_notify(tile[5]);
		}
	}

	if (is_coliding_tile_pair(tile[2], tile[3])) {
		obj->collision_state |= COLLISION_LEFT;
	}
	if (is_coliding_tile_pair(tile[4], tile[5])) {
		obj->collision_state |= COLLISION_RIGHT;
	}
	if (is_coliding_tile_pair(tile[0], tile[1])) {
		obj->collision_state |= COLLISION_UP;
	}

	if (is_coliding_tile_pair(tile[6], tile[7])) {
		obj->collision_state |= COLLISION_DOWN;
	}

	if (is_coliding_down_tile_pair(tile[6], tile[7])) {
		obj->collision_state |= COLLISION_DOWN_FT;
		if (notify) {
			phys_tile_collision_notify(tile[6]);
			phys_tile_collision_notify(tile[7]);
		}

		obj->ypos = (y / 8) * 8;
	}

}

/**
 * compute tile colision based on future position
 * XXX: Enemies do not trigger collision callbacks with tob, need a flag.
 */
static void phys_detect_tile_collisions_16x16(struct displ_object *obj,
        uint8_t *map, int8_t dx, int8_t dy, bool notify) __nonbanked
{
	uint8_t x,y, c;
	uint8_t *base_tl, *base_bl, *base_tr, *base_br;
	uint8_t *base_mr, *base_ml, *base_mt, *base_mb;

	x = obj->xpos + dx;
	y = obj->ypos + dy;

	base_tl = map + x / 8 + y / 8 * TILE_WIDTH;
	base_bl = map + x / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_ml = map + x / 8 + (y + 7) / 8 * TILE_WIDTH;
	base_tr = map + (x + 15) / 8 + y / 8 * TILE_WIDTH;
	base_br = map + (x + 15) / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_mr = map + (x + 15) / 8 + (y + 7) / 8 * TILE_WIDTH;
	base_mt = map + (x + 7) / 8 + y / 8 * TILE_WIDTH;
	base_mb = map + (x + 7) / 8 + (y + 15) / 8 * TILE_WIDTH;

	tile[0] = *(base_tl);
	tile[1] = *(base_ml);
	tile[2] = *(base_bl);
	tile[3] = *(base_tr);
	tile[4] = *(base_mr);
	tile[5] = *(base_br);
	tile[6] = *(base_mt);
	tile[7] = *(base_mb);

	obj->collision_state = 0;
	if (dx < 0 && is_coliding_tile_pair(tile[0], tile[1])) {
		obj->collision_state |= COLLISION_LEFT;
	}
	if (dx > 0 && is_coliding_tile_pair(tile[3], tile[4])) {
		obj->collision_state |= COLLISION_RIGHT;
	}
	if (dy < 0 && is_coliding_tile_triplet(tile[0], tile[3], tile[6])) {
		obj->collision_state |= COLLISION_UP;
	}
	if (dy > 0) {
		if ((dx >= 0 && is_coliding_tile_pair(tile[5], tile[7]))
			|| (dx < 0 && is_coliding_tile_pair(tile[2], tile[5])))
			obj->collision_state |= COLLISION_DOWN;
	}
}

/*
 * Update collision state of a display object 16x16
 */
void phys_detect_tile_collisions(struct displ_object *obj, uint8_t *map,
       int8_t dx, int8_t dy, bool notify) __nonbanked
{
       uint8_t size = obj->spr->pattern_set->size;

       if (size == SPR_SIZE_16x16) {
              phys_detect_tile_collisions_16x16(obj,map, dx, dy, notify);
       } else if (size = SPR_SIZE_16x32) {
              phys_detect_tile_collisions_16x32(obj, map, dx, dy, notify);
       }
}

void phys_detect_fall(struct displ_object *obj, uint8_t *map, int8_t dx) __nonbanked
{
	uint8_t size = obj->spr->pattern_set->size;

	uint8_t x,y;
	int8_t x_offset, y_offset;
	uint8_t *base_bl, *base_br;

	x = obj->xpos;
	y = obj->ypos;

	y_offset = 16;
	if (size == SPR_SIZE_16x32)
		y_offset = 32;

	x_offset = 8;
	if (dx < 0)
		x_offset = -8;

	base_bl = map + (x + x_offset) / 8 + (y + y_offset) / 8 * TILE_WIDTH;
	base_br = map + (x + x_offset + 15) / 8 + (y + y_offset) / 8 * TILE_WIDTH;

	tile[0] = *(base_bl);
	tile[1] = *(base_br);
	 //
	 // *(base_bl) = 9;
	 // *(base_br) = 9;

	obj->collision_state &= ~COLLISION_DOWN;
	if (is_coliding_tile_pair(tile[0], tile[1]))
		obj->collision_state |= COLLISION_DOWN;
}
