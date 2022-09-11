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

#include "tile.h"
#include "tileblit.h"
#include "bitmap.h"
#include "log.h"
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "mem.h"


#pragma CODE_PAGE 2

/**
 * Blit tile object with mask
 */
void tileblit_object_show(TileObject *tileobject, TileSet *background, uint8_t *scrbuf, bool refresh_vram) __nonbanked
{
  TileSet *ts = tileobject->tileset;
  uint8_t *old_pattern = ts->pattern;
  uint8_t *old_color = ts->color;
  uint16_t size = ts->frame_w * ts->frame_h * 8;
  uint16_t offset = tileobject->x / 8 + tileobject->y / 8 * 32;
  uint8_t tile;
  uint8_t *new_pat, *bg_pat;
  uint8_t *new_pat_color, *bg_color;
  uint8_t i, x, y, j, k;

  new_pat = mem_alloc(size);
  new_pat_color = mem_alloc(size);

  sys_memcpy(new_pat, ts->pattern, size);
  sys_memcpy(new_pat_color, ts->color, size);

  ts->pattern = new_pat;
  ts->color = new_pat_color;
  ts->allocated = false;

  k = 0;
  for (y = 0; y < ts->h; y++) {
    for (x = 0; x < ts->w; x++) {
      tile = *(scrbuf + offset);
      tile += background->pidx - 2;
      bg_pat = background->pattern + tile * 8;
      bg_color = background->color + tile * 8;
      for (i = 0; i < 8; i++, k++) {
        if ((new_pat_color[k] & 0x0f) == 0) {
          if (new_pat[k] != 0xff) {
              new_pat[k] = ~new_pat[k] & bg_pat[i];
              if (new_pat[k] == 0) {
                new_pat_color[k] = bg_color[i];
              } else {
                new_pat_color[k] = bg_color[i] & 0xf0;
              }
          }
        } 
      }
      offset++;
    }
    offset += 32 - ts->w;
  }

  tile_set_valloc(ts);
  tile_object_show(tileobject, scrbuf, refresh_vram);

  ts->pattern = old_pattern;
  ts->color = old_color;

  mem_free(new_pat);
  mem_free(new_pat_color);
}
