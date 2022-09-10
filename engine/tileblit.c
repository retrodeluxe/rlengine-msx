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

/*
 * tileblit for integer tile positions
 */
static void tileblit_object_mask(uint8_t *dst, uint8_t *src, uint8_t w, uint8_t h) __nonbanked
{
  unused(dst);
  unused(src);
  unused(h);

    __asm
    ld  c,8(ix)  ; height
    exx
    ld  c,4(ix)  ; dst
  	ld  b,5(ix)
    ld  e,6(ix)  ; src
  	ld  d,7(ix)
    ld  hl, #32
    add hl, de
    exx
la:
    ld  b,#32 ; width 4 
lb:
    exx
    ld  a,(bc)  ; destinatom
    or (hl)    ; and mask (mask must be 15 on the transparent areas)
    ex  de,hl
    and  (hl)    ; or pattern (pattern is 15 on the visible areas)
    ex  de,hl
    ld  (bc),a
    inc bc
    inc de
    inc hl
    exx
    djnz lb

    ; after one row, need to increase dest by 256 - 32 and source by 32
    exx
    push hl
    push de
    ld h, b
    ld l, c
    ld de, #224  // 256 - 32 this depends on the viewport size 
    add hl,de
    ld b, h
    ld c, l
    pop de
    pop hl
    push hl
    push bc
    ld h, d
    ld l, e
    ld de, #32
    add hl,de
    ld d, h
    ld e, l
    pop bc
    pop hl
    ld  hl, #32
    add hl, de
    exx
    dec c
    jr nz,la
    __endasm;
}

/**
 * Blit tile object with mask
 */
void tileblit_object_show(TileObject *tileobject, TileSet *background, uint8_t *scrbuf, bool refresh_vram) __nonbanked
{
  TileSet *ts = tileobject->tileset;
  uint8_t *pattern = ts->pattern;
  uint8_t *color = ts->color;
  uint16_t size = ts->frame_w * ts->frame_h * 8;
  uint8_t i, x, y, k;

  uint16_t offset = tileobject->x / 8 + tileobject->y / 8 * 32;
  uint8_t *ptr = scrbuf + offset;
  uint8_t tile_base = tileobject->tileset->pidx;
  uint8_t tile;

  uint8_t *new_pat, *bg_pat;
  uint8_t *new_pat_color, *bg_color;

  new_pat = mem_alloc(size);
  new_pat_color = mem_alloc(size);

  ts->pattern = new_pat;
  ts->color = new_pat_color;

  sys_memcpy(new_pat, pattern, size);
  sys_memcpy(new_pat_color, color, size);

  // log_i("source\n");
  // for (i = 0; i < 8; i++) {
  //   log_i("pat %d %x\n", i,  *new_pat++);
  //   log_i("col %d %x\n", i, *new_pat_color++);
  // }

   k = 0;
   for (y = 0; y < ts->h; y++) {
     for (x = 0; x < ts->w; x++) {
        tile = *(scrbuf + offset);
        tile += background->pidx - 2;
        log_i("tile %d\n", tile);
        bg_pat = background->pattern + tile * 8;
        bg_color = background->color + tile * 8;
        for (i = 0; i < 8; i++, k++) {
          if ((new_pat_color[k] & 0x0f) == 0) {
            new_pat[k] = ~new_pat[k] & bg_pat[i];
            new_pat_color[k] = bg_color[i]; 
          } else if(new_pat_color[i] & 0xf0 == 0) {
            new_pat[i] |= bg_pat[i];
          }
        }
        offset++;
     }
     offset+=32;
   }

  tile_set_valloc(ts);
  tile_object_show(tileobject, scrbuf, refresh_vram);

  mem_free(new_pat);
  mem_free(new_pat_color);
}
