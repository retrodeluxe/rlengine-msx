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
#include "msx.h"

#ifndef _TILE_H_
#define _TILE_H_

/**
 * Name Table Bank in `MODE_GRP2`
 */
typedef enum {
  /**
   * Top Bank
   */
  BANK0,
  /**
   * Middle Bank
   */
  BANK1,
  /**
   * Bottom Bank
   */
  BANK2,
  /**
   * All Banks
   */
  ALLBANKS
} TileBank;

/**
 * Defines a TileSet
 */
typedef struct TileSet TileSet;

/**
 * Contents of a TileSet
 */
struct TileSet {
  /**
   * TileSet width in pixels
   */
  uint8_t w;
  /**
   * TileSet height in pixels
   */
  uint8_t h;
  /**
   * Pointer to the TileSet pattern data
   */
  uint8_t *pattern;
  /**
   * Pointer to the TileSet color data
   */
  uint8_t *color;
  /**
   * Offset of the TileSet once allocated into VRAM (0 to 255)
   */
  uint8_t pidx;
  /**
   * True if the tileset is allocated in VRAM
   */
  bool allocated;
  /**
   * True if the TileSet data is not compressed
   */
  bool raw;
  /**
   * Animation frame width in number tiles
   */
  uint8_t frame_w;
  /**
   * Animation frame height in number of tiles
   */
  uint8_t frame_h;
  /**
   * Number of animation frames within the set
   */
  uint8_t frames;
  /**
   * Number of animation states within the set
   */
  uint8_t states;
};

/**
 * Defines a TileMap
 */
typedef struct TileMap TileMap;

/**
 * Contents of a TileMap
 */
struct TileMap {
  uint16_t cur_x;
  uint16_t cur_y;
  uint8_t w;
  uint8_t h;
  uint8_t *map;
};

/**
 * Defines a TileObject
 */
typedef struct TileObject TileObject;

/**
 * Contens of a TileObject
 */
struct TileObject {
  /**
   * Screen X position in pixel coordinates
   */
  uint8_t x;
  /**
   * Screen Y position in pixel coordinates
   */
  uint8_t y;
  /**
   * Current animation state
   */
  uint8_t state;
  /**
   * Current animation frame
   */
  uint8_t frame;
  /**
   * Tileset data
   */
  TileSet *tileset;
  uint8_t idx;
};

 /**
 * Initialize a Static TileSet
 *
 * :param TS: a TileSet object
 * :param DATA: name of data asset
 */
#define INIT_TILE_SET(TS, DATA)                                                \
  (TS).w = DATA##_tile_w;                                                      \
  (TS).h = DATA##_tile_h;                                                      \
  (TS).pattern = DATA##_tile;                                                  \
  (TS).color = DATA##_tile_color;                                              \
  (TS).allocated = false;                                                      \
  (TS).raw = false;

/**
 * Initialize an uncompressed Static TileSet
 *
 * :param TS: a TileSet object
 * :param DATA: name of data asset
 */
#define INIT_RAW_TILE_SET(TS, DATA)                                            \
  (TS).w = DATA##_tile_w;                                                      \
  (TS).h = DATA##_tile_h;                                                      \
  (TS).pattern = DATA##_tile;                                                  \
  (TS).color = DATA##_tile_color;                                              \
  (TS).allocated = false;                                                      \
  (TS).raw = true;

 /**
 * Initialize a Dynamic TileSet
 *
 * :param TS: a TileSet object
 * :param DATA: name of data asset
 * :param W: frame width of the tileset in tile units
 * :param H: frame heigth of the tileset in tile units
 * :param F: number of frames per state
 * :param S: number of states
 */
#define INIT_DYNAMIC_TILE_SET(TS, DATA, W, H, F, S)                            \
  (TS).w = DATA##_tile_w;                                                      \
  (TS).h = DATA##_tile_h;                                                      \
  (TS).pattern = DATA##_tile;                                                  \
  (TS).color = DATA##_tile_color;                                              \
  (TS).allocated = false;                                                     \
  (TS).frame_w = W;                                                           \
  (TS).frame_h = H;                                                           \
  (TS).frames = F;                                                          \
  (TS).states = S;                                                            \
  (TS).raw = false;

 /**
 * Initialize a Dynamic TileSet
 *
 * :param TS: a TileSet object
 * :param DATA: name of data asset
 * :param W: frame width of the tileset in tile units
 * :param H: frame heigth of the tileset in tile units
 * :param F: number of frames
 * :param S: number of states
 */
 #define INIT_RAW_DYNAMIC_TILE_SET(TS, DATA, W, H, F, S)                        \
  (TS).w = DATA##_tile_w;                                                      \
  (TS).h = DATA##_tile_h;                                                      \
  (TS).pattern = DATA##_tile;                                                  \
  (TS).color = DATA##_tile_color;                                              \
  (TS).allocated = false;                                                     \
  (TS).frame_w = W;                                                           \
  (TS).frame_h = H;                                                           \
  (TS).frames = F;                                                          \
  (TS).states = S;                                                            \
  (TS).raw = true;

extern void tile_init();
extern rle_result tile_set_valloc(TileSet *tileset);
extern rle_result tile_set_valloc_bank(TileSet *tileset, TileBank bank);
extern void tile_set_vfree(TileSet *tileset);
extern void tile_set_to_vram_bank(TileSet *tileset, TileBank bank, uint8_t offset);
extern void tile_set_to_vram(TileSet *tileset, uint8_t offset);
extern void tile_set_to_vram_bank_raw(TileSet *tileset, TileBank bank,
                                      uint8_t offset);
extern void tile_set_to_vram_raw(TileSet *tileset, uint8_t offset);
extern void tile_object_show(TileObject *tileobject, uint8_t *scrbuf,
                             bool refresh_vram) __nonbanked;
extern void tile_object_hide(TileObject *tileobject, uint8_t *scrbuf,
                             bool refresh_vram) __nonbanked;
#endif
