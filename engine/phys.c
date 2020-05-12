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

static uint8_t colliding_tiles[32];
static uint8_t colliding_tiles_down[32];
static uint8_t tile[12];
static void (*sprite_colision_cb)();

static struct tile_collision_group cgroup[MAX_CROUPS];
static uint8_t n_cgroups;

#define STATFL 0xf3e7;
#define SPR_COLISION_MASK 32
#define SPR_5TH_ALIGN_MASK 64

void phys_check_collision_bit()
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
	n_cgroups = 0;
}

/* note that this runs in interrupt context */
void phys_set_sprite_collision_handler(void (*handler))
{
	sprite_colision_cb = handler;
	sys_irq_register(phys_check_collision_bit);
}

/**
 * set callbacks for specific tiles
 */
void phys_set_tile_collision_handler(struct displ_object *dpo, void (*handler), uint8_t data)
{
	uint8_t base_tile = dpo->tob->ts->pidx;
	uint8_t num_tiles = dpo->tob->ts->frame_w * dpo->tob->ts->frame_h *
		dpo->tob->ts->n_frames * dpo->tob->ts->n_dirs;

	cgroup[n_cgroups].start = base_tile;
	cgroup[n_cgroups].end = base_tile + num_tiles - 1;
	cgroup[n_cgroups].handler = handler;
	cgroup[n_cgroups].data = data;
	cgroup[n_cgroups].dpo = dpo;
	//log_e("adding tile colision handler start %d, end %d\n", base_tile, base_tile + num_tiles);
	n_cgroups++;
}

/**
 * Sets all the tiles composing an object as coliding tiles
 */
void phys_set_colliding_tile_object(struct displ_object *dpo, bool down)
{
	uint8_t i;
	uint8_t base_tile = dpo->tob->ts->pidx;
	uint8_t num_tiles = dpo->tob->ts->frame_w * dpo->tob->ts->frame_h *
		dpo->tob->ts->n_frames * dpo->tob->ts->n_dirs;

	for (i = base_tile; i < base_tile + num_tiles; i++) {
		if (down)
			phys_set_down_colliding_tile(i);
		else
			phys_set_colliding_tile(i);
	}
}

/*
 * if the tile has a handler set, notify
 */
static void phys_tile_collision_notify(uint8_t tile)
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
}

/**
 * Set a tile that collides only when falling
 */
void phys_set_down_colliding_tile(uint8_t tile)
{
        bitmap_reset(colliding_tiles_down, tile);
}

void phys_clear_colliding_tile(uint8_t tile)
{
        bitmap_set(colliding_tiles, tile);
        bitmap_set(colliding_tiles_down, tile);
}

static bool is_coliding_tile_pair(uint8_t tile1, uint8_t tile2)
{
	return ((bitmap_get(colliding_tiles, tile1) == 0) ||
                 (bitmap_get(colliding_tiles, tile2) == 0));
}

static bool is_coliding_down_tile_pair(uint8_t tile1, uint8_t tile2)
{
	return ((bitmap_get(colliding_tiles_down, tile1) == 0) ||
                 (bitmap_get(colliding_tiles_down, tile2) == 0));
}

static bool is_coliding_tile_triplet(uint8_t tile1, uint8_t tile2, uint8_t tile3)
{
	return ((bitmap_get(colliding_tiles, tile1) == 0) ||
                 (bitmap_get(colliding_tiles, tile2) == 0) ||
                 (bitmap_get(colliding_tiles, tile3) == 0));
}


#define TILE_WIDTH 32


/**
 * Update dpo collision_state
 */
static void phys_detect_tile_collisions_16x32(struct displ_object *obj,
        uint8_t *map, int8_t dx, int8_t dy)
{
	uint8_t x, y, c;
	uint8_t *base_tl, *base_bl, *base_tr, *base_br;
	uint8_t *base_mr, *base_ml, *base_mt, *base_mb;

	x = obj->xpos + dx;
	y = obj->ypos + dy;

	base_tl = map + x / 8 + y / 8 * TILE_WIDTH;
	base_bl = map + x / 8 + (y + 31) / 8 * TILE_WIDTH;
	base_ml = map + x / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_tr = map + (x + 15) / 8 + y / 8 * TILE_WIDTH;
	base_br = map + (x + 15) / 8 + (y + 31) / 8 * TILE_WIDTH;
	base_mr = map + (x + 15) / 8 + (y + 15) / 8 * TILE_WIDTH;
	base_mt = map + (x + 7) / 8 + y / 8 * TILE_WIDTH;
	base_mb = map + (x + 7) / 8 + (y + 31) / 8 * TILE_WIDTH;

	tile[0] = *(base_tl);
	tile[1] = *(base_ml);
	tile[2] = *(base_bl);
	tile[3] = *(base_tr);
	tile[4] = *(base_mr);
	tile[5] = *(base_br);
	tile[6] = *(base_mt);
	tile[7] = *(base_mb);

	obj->collision_state = 0;
	if (dx < 0 && is_coliding_tile_triplet(tile[0], tile[1], tile[2])) {
		obj->collision_state |= COLLISION_LEFT;
		phys_tile_collision_notify(tile[0]);
		phys_tile_collision_notify(tile[1]);
		phys_tile_collision_notify(tile[2]);
	}
	if (dx > 0 && is_coliding_tile_triplet(tile[3], tile[4], tile[5])) {
		obj->collision_state |= COLLISION_RIGHT;
		phys_tile_collision_notify(tile[3]);
		phys_tile_collision_notify(tile[4]);
		phys_tile_collision_notify(tile[5]);
	}
	if (dy < 0 && is_coliding_tile_triplet(tile[0], tile[3], tile[6])) {
		obj->collision_state |= COLLISION_UP;
		phys_tile_collision_notify(tile[0]);
		phys_tile_collision_notify(tile[3]);
		phys_tile_collision_notify(tile[6]);
	}
	if (dy > 0) {
		if ((dx >= 0 && is_coliding_tile_pair(tile[5], tile[7]))
			|| (dx < 0 && is_coliding_tile_pair(tile[2], tile[5])))
			obj->collision_state |= COLLISION_DOWN;
	}
}

/**
 * compute tile colision based on future position
 *
 */
static void phys_detect_tile_collisions_16x16(struct displ_object *obj,
        uint8_t *map, int8_t dx, int8_t dy)
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

	 // *(base_tr) = 0;
	 // *(base_mr) = 0;
	 // *(base_br) = 0;
	 // *(base_bl) = 0;
	 // *(base_tl) = 0;
	 // *(base_ml) = 0;
	 // *(base_mt) = 0;
	 // *(base_mb) = 0;

	// XXX: Collision logic can be tuned more based on dx, dy

	obj->collision_state = 0;
	if (dx < 0 && is_coliding_tile_pair(tile[0], tile[1])) {
		obj->collision_state |= COLLISION_LEFT;
		phys_tile_collision_notify(tile[0]);
		phys_tile_collision_notify(tile[1]);
	}
	if (dx > 0 && is_coliding_tile_pair(tile[3], tile[4])) {
		obj->collision_state |= COLLISION_RIGHT;
		phys_tile_collision_notify(tile[3]);
		phys_tile_collision_notify(tile[4]);
	}
	if (dy < 0 && is_coliding_tile_triplet(tile[0], tile[3], tile[6])) {
		obj->collision_state |= COLLISION_UP;
		phys_tile_collision_notify(tile[0]);
		phys_tile_collision_notify(tile[3]);
		phys_tile_collision_notify(tile[6]);
	}
	if (dy > 0) {
		if ((dx >= 0 && is_coliding_tile_pair(tile[5], tile[7]))
			|| (dx < 0 && is_coliding_tile_pair(tile[2], tile[5])))
			obj->collision_state |= COLLISION_DOWN;
			// phys_tile_collision_notify(tile[5]);
			// phys_tile_collision_notify(tile[7]);
			// phys_tile_collision_notify(tile[2]);
			// phys_tile_collision_notify(tile[5]);
	}
}

/*
 * Update collision state of a display object 16x16
 */
void phys_detect_tile_collisions(struct displ_object *obj, uint8_t *map,
       int8_t dx, int8_t dy)
{
       uint8_t size = obj->spr->pattern_set->size;

       if (size == SPR_SIZE_16x16) {
              phys_detect_tile_collisions_16x16(obj,map, dx, dy);
       } else if (size = SPR_SIZE_16x32) {
              phys_detect_tile_collisions_16x32(obj, map, dx, dy);
       }
}
