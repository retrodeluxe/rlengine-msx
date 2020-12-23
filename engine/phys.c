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

#include "phys.h"
#include "bitmap.h"
#include "dpo.h"
#include "log.h"
#include "msx.h"
#include "sprite.h"
#include "sys.h"
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

#define SPR_COLLISION_FRAME_SKIP 7 /* bitmask for frameskip */

static uint8_t frame_skip;
static bool skip;

void phys_check_collision_bit() __nonbanked {
  uint8_t *status = (uint8_t *)STATFL;

  /** check collisions every X frames **/
  if (!frame_skip && (*status & SPR_COLISION_MASK) != 0) {
    sprite_colision_cb();
    skip = true;
  }
  if (skip) {
    frame_skip++;
    frame_skip &= SPR_COLLISION_FRAME_SKIP;
    if (!frame_skip)
      skip = false;
  }
}

void phys_init() {
  sys_memset(colliding_tiles, 255, 32);
  sys_memset(colliding_tiles_down, 255, 32);
  sys_memset(colliding_tiles_trigger, 255, 32);
  sprite_colision_cb = NULL;
  n_cgroups = 0;
  frame_skip = 0;
  skip = false;
}

/* note that this runs in interrupt context */
void phys_set_sprite_collision_handler(void(*handler)()) {
  if (sprite_colision_cb == NULL) {
    sprite_colision_cb = handler;
    sys_irq_register(phys_check_collision_bit);
  }
}

void phys_clear_sprite_collision_handler() __nonbanked {
  if (sprite_colision_cb != NULL) {
    sys_irq_unregister(phys_check_collision_bit);
    sprite_colision_cb = NULL;
  }
}

/**
 * set callbacks for specific tiles
 */
void phys_set_tile_collision_handler(enum tile_collision_type type,
          DisplayObject *dpo, void(*handler)(DisplayObject *dpo, uint8_t data),
                                     uint8_t data) {
  uint8_t i;
  uint8_t base_tile = dpo->tob->tileset->pidx;
  uint8_t num_tiles = dpo->tob->tileset->frame_w * dpo->tob->tileset->frame_h *
                      dpo->tob->tileset->frames * dpo->tob->tileset->states;

  for (i = 0; i < n_cgroups; i++) {
    if (cgroup[i].start == base_tile) {
      break;
    }
  }

  if (i < n_cgroups && !(type & TILE_COLLISION_MULTIPLE))
    return;

  cgroup[n_cgroups].start = base_tile;
  cgroup[n_cgroups].end = base_tile + num_tiles - 1;
  cgroup[n_cgroups].handler = handler;
  cgroup[n_cgroups].data = data;
  cgroup[n_cgroups].dpo = dpo;
  cgroup[n_cgroups].type = type;
  n_cgroups++;

  // dump cgroups
  // for (i = 0; i < n_cgroups; i++) {
  //	log_e("cgroup %d: start %d end %d\n", i, cgroup[i].start,cgroup[i].end);
  //}
}

/**
 * Sets all the tiles composing an object as coliding tiles
 */
void phys_set_colliding_tile_object(DisplayObject *dpo,
                                    enum tile_collision_type type,
                                    void(*handler)(DisplayObject *dpo, uint8_t data), uint8_t data) {
  uint8_t i;
  uint8_t base_tile = dpo->tob->tileset->pidx;
  uint8_t num_tiles = dpo->tob->tileset->frame_w * dpo->tob->tileset->frame_h *
                      dpo->tob->tileset->frames * dpo->tob->tileset->states;

  for (i = base_tile; i < base_tile + num_tiles; i++) {
    if (type & TILE_COLLISION_DOWN)
      phys_set_down_colliding_tile(i);
    else if (type & TILE_COLLISION_TRIGGER)
      phys_set_trigger_colliding_tile(i);
    else if (type & TILE_COLLISION_FULL)
      phys_set_colliding_tile(i);
  }

  phys_set_tile_collision_handler(type, dpo, handler, data);
}

void phys_set_masked_colliding_tile_object(DisplayObject *dpo,
                                           enum tile_collision_type type,
                                           uint8_t x, uint8_t y, uint8_t w,
                                           uint8_t h, void(*handler)(DisplayObject *dpo, uint8_t data),
                                           uint8_t data) {
  uint8_t i, j, k;
  uint8_t base_tile = dpo->tob->tileset->pidx;
  uint8_t num_tiles = dpo->tob->tileset->frame_w * dpo->tob->tileset->frame_h *
                      dpo->tob->tileset->frames * dpo->tob->tileset->states;
  uint8_t tiles_in_row =
      dpo->tob->tileset->frame_w * dpo->tob->tileset->frames * dpo->tob->tileset->states;
  uint8_t num_frames = dpo->tob->tileset->frames * dpo->tob->tileset->states;
  uint8_t frame_base;

  for (i = 0; i < num_frames; i++) {
    frame_base = base_tile + i * dpo->tob->tileset->frame_w;
    frame_base += x + y * tiles_in_row;
    for (j = frame_base; j < frame_base + h * tiles_in_row; j++) {
      for (k = j; k < j + w; k++) {
        if (type & TILE_COLLISION_DOWN)
          phys_set_down_colliding_tile(k);
        else if (type & TILE_COLLISION_TRIGGER)
          phys_set_trigger_colliding_tile(k);
        else if (type & TILE_COLLISION_FULL)
          phys_set_colliding_tile(k);
      }
    }
  }

  phys_set_tile_collision_handler(type, dpo, handler, data);
}

void phys_clear_colliding_tile_object(DisplayObject *dpo) {
  uint8_t i;
  uint8_t base_tile = dpo->tob->tileset->pidx;
  uint8_t num_tiles = dpo->tob->tileset->frame_w * dpo->tob->tileset->frame_h *
                      dpo->tob->tileset->frames * dpo->tob->tileset->states;

  for (i = base_tile; i < base_tile + num_tiles; i++) {
    phys_clear_colliding_tile(i);
  }

  // TODO: clear the collision group as well
}

/**
 * registers tiles from a plain tile_set as colliding
 */
void phys_set_colliding_tile_set(TileSet *ts) {
  uint8_t i;
  uint8_t base_tile = ts->pidx;
  uint8_t num_tiles = ts->w * ts->h;

  for (i = base_tile; i < base_tile + num_tiles; i++) {
    phys_set_colliding_tile(i);
  }
}

/*
 * if the tile has a handler set, notify
 */
static void phys_tile_collision_notify(uint8_t tile, uint16_t x,
                                       uint16_t y) __nonbanked {
  uint8_t i;
  int16_t d;

  unused(y);

  for (i = 0; i < n_cgroups; i++) {
    if (tile >= cgroup[i].start && tile <= cgroup[i].end) {
      if (cgroup[i].type & TILE_COLLISION_MULTIPLE) {
        d = (cgroup[i].dpo->xpos - x);
        d = d < 0 ? (d * -1) : d;
        if (d > 32) {
          log_e("bad obj: %d\n", d);
          continue;
        }
      }
      // HACK: this only works with game_test at the moment
      //       called from anim(6) but the handlers are in logic(3)
      ascii8_set_code(3);
      cgroup[i].handler(cgroup[i].dpo, cgroup[i].data);
      ascii8_set_code(6);
    }
  }
}
/*
 * Set collision flag for a specific tile (all directions)
 */
void phys_set_colliding_tile(uint8_t tile) {
  bitmap_reset(colliding_tiles, tile);
  bitmap_reset(colliding_tiles_down, tile);
  bitmap_reset(colliding_tiles_trigger, tile);
}

/**
 * Set a tile that collides only when falling
 */
void phys_set_down_colliding_tile(uint8_t tile) {
  bitmap_reset(colliding_tiles_down, tile);
}

void phys_set_trigger_colliding_tile(uint8_t tile) {
  bitmap_reset(colliding_tiles_trigger, tile);
}

void phys_clear_colliding_tile(uint8_t tile) {
  bitmap_set(colliding_tiles, tile);
  bitmap_set(colliding_tiles_down, tile);
  bitmap_set(colliding_tiles_trigger, tile);
}

static bool is_coliding_tile_pair(uint8_t tile1, uint8_t tile2) __nonbanked {
  return ((bitmap_get(colliding_tiles, tile1) == 0) ||
          (bitmap_get(colliding_tiles, tile2) == 0));
}

static bool is_coliding_down_tile_pair(uint8_t tile1,
                                       uint8_t tile2) __nonbanked {
  return ((bitmap_get(colliding_tiles_down, tile1) == 0) ||
          (bitmap_get(colliding_tiles_down, tile2) == 0));
}

static bool is_coliding_tile_triplet(uint8_t tile1, uint8_t tile2,
                                     uint8_t tile3) __nonbanked {
  return ((bitmap_get(colliding_tiles, tile1) == 0) ||
          (bitmap_get(colliding_tiles, tile2) == 0) ||
          (bitmap_get(colliding_tiles, tile3) == 0));
}

static bool is_coliding_trigger_tile_pair(uint8_t tile1,
                                          uint8_t tile2) __nonbanked {
  return ((bitmap_get(colliding_tiles_trigger, tile1) == 0) ||
          (bitmap_get(colliding_tiles_trigger, tile2) == 0));
}

#define TILE_WIDTH 32

/**
 * Update dpo collision_state
 */
static void phys_detect_tile_collisions_16x32(DisplayObject *obj, uint8_t *map,
                                              int8_t dx, int8_t dy, bool duck,
                                              bool notify) __nonbanked {
  int16_t x, y;
  int16_t xp, yp;
  uint8_t *base_ceiling_l, *base_ceiling_r, *base_left_t, *base_left_b;
  uint8_t *base_right_t, *base_right_b, *base_floor_l, *base_floor_r;
  uint8_t *base_inner_t, *base_inner_b;

  xp = obj->xpos + dx;
  yp = obj->ypos + dy;

  /** truncate tile positions to screen borders **/
  /** and add logic to handle corner cases **/
  if (xp < 0)
    x = 0;
  else if (xp > 240)
    x = 240;
  else
    x = xp;
  if (dy < 0 && yp < -12)
    y = -12;
  else if (yp > 143)
    y = 143;
  else
    y = yp;

  base_ceiling_l = map + (x + 4) / 8 + (y + 16) / 8 * TILE_WIDTH;
  base_ceiling_r = map + (x + 8) / 8 + (y + 16) / 8 * TILE_WIDTH;

  base_left_b = map + x / 8 + (y + 24) / 8 * TILE_WIDTH;
  base_right_b = map + (x + 12) / 8 + (y + 24) / 8 * TILE_WIDTH;

  if (duck) {
    base_left_t = base_left_b;
    base_right_t = base_right_b;
  } else {
    base_left_t = map + x / 8 + (y + 18) / 8 * TILE_WIDTH;
    base_right_t = map + (x + 12) / 8 + (y + 18) / 8 * TILE_WIDTH;
  }

  base_floor_l = map + (x + 4) / 8 + (y + 32) / 8 * TILE_WIDTH;
  base_floor_r = map + (x + 8) / 8 + (y + 32) / 8 * TILE_WIDTH;

  base_inner_t = map + (x + 7) / 8 + (y + 18) / 8 * TILE_WIDTH;
  base_inner_b = map + (x + 7) / 8 + (y + 24) / 8 * TILE_WIDTH;

  tile[0] = *(base_ceiling_l);
  tile[1] = *(base_ceiling_r);
  tile[2] = *(base_left_t);
  tile[3] = *(base_left_b);
  tile[4] = *(base_right_t);
  tile[5] = *(base_right_b);
  tile[6] = *(base_floor_l);
  tile[7] = *(base_floor_r);
  tile[8] = *(base_inner_t);
  tile[9] = *(base_inner_b);

  // vdp_write(VRAM_BASE_NAME + base_ceiling_l - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_ceiling_r - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_left_t - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_left_b - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_right_t - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_right_b - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_floor_l - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_floor_r - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_inner_t - map, 254);
  // vdp_write(VRAM_BASE_NAME + base_inner_b - map, 254);

  obj->collision_state = 0;

  /** check non-blocking collisions **/
  if (notify) {
    if (is_coliding_trigger_tile_pair(tile[8], tile[9])) {
      phys_tile_collision_notify(tile[9], x, y);
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

  if (is_coliding_down_tile_pair(tile[6], tile[7]) && dy >= 0) {
    obj->collision_state |= COLLISION_DOWN_FT;
    if (notify) {
      phys_tile_collision_notify(tile[6], x, y);
      phys_tile_collision_notify(tile[7], x, y);
    }

    if (dy > 0 && yp < 0)
      obj->ypos = (int16_t)(((y / 8) - 1) * 8);
    else
      obj->ypos = (int16_t)((y / 8) * 8);
  }
}

/**
 * compute tile colision based on future position
 * XXX: Enemies do not trigger collision callbacks with tob, need a flag.
 */
static void phys_detect_tile_collisions_16x16(DisplayObject *obj, uint8_t *map,
                                              int8_t dx, int8_t dy, bool duck,
                                              bool notify) __nonbanked {
  uint8_t x, y;
  uint8_t *base_tl, *base_bl, *base_tr, *base_br;
  uint8_t *base_mr, *base_ml, *base_mt, *base_mb;

  unused(duck);
  unused(notify);

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
    if ((dx >= 0 && is_coliding_tile_pair(tile[5], tile[7])) ||
        (dx < 0 && is_coliding_tile_pair(tile[2], tile[5])))
      obj->collision_state |= COLLISION_DOWN;
  }
}

/*
 * Update collision state of a display object 16x16
 */
void phys_detect_tile_collisions(DisplayObject *obj, uint8_t *map, int8_t dx,
                                 int8_t dy, bool duck,
                                 bool notify) __nonbanked {
  uint8_t size = obj->spr->pattern_set->size;

  if (size == SPR_SIZE_16x16) {
    phys_detect_tile_collisions_16x16(obj, map, dx, dy, duck, notify);
  } else if (size = SPR_SIZE_16x32) {
    phys_detect_tile_collisions_16x32(obj, map, dx, dy, duck, notify);
  }
  // else if (size = SPR_SIZE_32x16) {
  //	phys_detect_tile_collisions_16x16(obj,map, dx, dy, duck, notify);
  //}
}
