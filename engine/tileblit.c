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
    ld  b,#32
lb:
    exx
    ld  a,(bc)
    and (hl)    ; and mask (mask must be 15 on the transparent areas)
    ex  de,hl
    or  (hl)    ; or pattern (pattern is 15 on the visible areas)
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
void tileblit_object_show(TileObject *tileobject, uint8_t *buffer) __nonbanked
{
  TileSet *ts = tileobject->tileset;
  uint8_t *pattern = ts->pattern;
  uint8_t *color = ts->color;
  uint16_t size = ts->frame_w * ts->frame_h * 8;

  uint16_t offset = (tileobject->x / 8) * 8 + (tileobject->y / 8) * 8 * 32 + tileobject->y % 8;
  tileblit_object_mask(buffer+offset, pattern, 4, 4);
}
