/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017-2021 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#ifndef _TILEBLIT_H_
#define _TILEBLIT_H_


 /**
 * Initialize an Blitable TileSet
 *
 * :param TS: a TileSet object
 * :param DATA: name of data asset
 * :param W: frame width of the tileset in tile units
 * :param H: frame heigth of the tileset in tile units
 * :param F: number of frames per state
 * :param S: number of states
 */
#define INIT_BLIT_TILE_SET(TS, DATA, W, H, F, S)                            \
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

void tileblit_object_show(TileObject *tileobject,  TileSet *background, uint8_t *buffer, bool refresh_vram) __nonbanked;


#endif /* TILEBLIT_H */
