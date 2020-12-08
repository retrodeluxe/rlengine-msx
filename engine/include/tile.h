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
#include <stdint.h>
#include <stdbool.h>

#ifndef _TILE_H_
#define _TILE_H_

#define gfx_screen_tile_w 32
#define gfx_screen_tile_h 24

#define BANK0 0
#define BANK1 1
#define BANK2 2
#define ALLBANKS 3

typedef struct TileSet TileSet;
struct TileSet {
  uint8_t w;
  uint8_t h;
  uint8_t *pattern;
  uint8_t *color;
  uint8_t pidx; /*< index of this tileset in vram pattern bank */
  bool allocated;
  /* dynamic tile data */
  uint8_t frame_w; /*< width of each tile object inside the set */
  uint8_t frame_h; /*< height of each tile object inside the set */
  uint8_t n_frames;
  uint8_t n_dirs;
  bool raw; /** indicates if the tileset is compressed */
};

typedef struct TileMap TileMap;
struct TileMap {
  uint16_t cur_x;
  uint16_t cur_y;
  uint8_t w;
  uint8_t h;
  uint8_t *map;
};

typedef struct TileObject TileObject;
struct TileObject {
  uint8_t x;
  uint8_t y;
  uint8_t cur_dir;
  uint8_t cur_anim_step;
  TileSet *ts;
  uint8_t idx; /*< index of first tile in the tile set */
};

#define INIT_TILE_SET(SET, GEN)                                                \
  (SET).w = GEN##_tile_w;                                                      \
  (SET).h = GEN##_tile_h;                                                      \
  (SET).pattern = GEN##_tile;                                                  \
  (SET).color = GEN##_tile_color;                                              \
  (SET).allocated = false;                                                     \
  (SET).raw = false;

#define INIT_RAW_TILE_SET(SET, GEN)                                            \
  (SET).w = GEN##_tile_w;                                                      \
  (SET).h = GEN##_tile_h;                                                      \
  (SET).pattern = GEN##_tile;                                                  \
  (SET).color = GEN##_tile_color;                                              \
  (SET).allocated = false;                                                     \
  (SET).raw = true;

#define INIT_DYNAMIC_TILE_SET(SET, GEN, W, H, F, D)                            \
  (SET).w = GEN##_tile_w;                                                      \
  (SET).h = GEN##_tile_h;                                                      \
  (SET).pattern = GEN##_tile;                                                  \
  (SET).color = GEN##_tile_color;                                              \
  (SET).allocated = false;                                                     \
  (SET).frame_w = W;                                                           \
  (SET).frame_h = H;                                                           \
  (SET).n_frames = F;                                                          \
  (SET).n_dirs = D;                                                            \
  (SET).raw = false;

#define INIT_RAW_DYNAMIC_TILE_SET(SET, GEN, W, H, F, D)                        \
  (SET).w = GEN##_tile_w;                                                      \
  (SET).h = GEN##_tile_h;                                                      \
  (SET).pattern = GEN##_tile;                                                  \
  (SET).color = GEN##_tile_color;                                              \
  (SET).allocated = false;                                                     \
  (SET).frame_w = W;                                                           \
  (SET).frame_h = H;                                                           \
  (SET).n_frames = F;                                                          \
  (SET).n_dirs = D;                                                            \
  (SET).raw = true;

extern void tile_init();
extern bool tile_set_valloc(TileSet *ts);
extern void tile_set_vfree(TileSet *ts);
extern void tile_set_to_vram_bank(TileSet *ts, uint8_t bank, uint8_t offset);
extern void tile_set_to_vram(TileSet *ts, uint8_t pos);
extern void tile_set_to_vram_bank_raw(TileSet *ts, uint8_t bank,
                                      uint8_t offset);
extern void tile_set_to_vram_raw(TileSet *ts, uint8_t pos);
extern void tile_map_clip(TileMap *tm, struct gfx_viewport *vp, uint8_t *scrbuf,
                          struct gfx_map_pos *p);

extern void tile_object_show(TileObject *to, uint8_t *scrbuf,
                             bool refresh_vram) __nonbanked;
extern void tile_object_hide(TileObject *to, uint8_t *scrbuf,
                             bool refresh_vram) __nonbanked;
#define set_tile_vram(_x, _y, _tile)                                           \
  vdp_poke(vdp_base_names_grp1 + 32 * _y + _x, _tile)

#endif
