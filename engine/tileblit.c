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

  // TODO: implement different w (3, 4)

  // TODO: implement blit rotation to support non-aligned sprites
  // that part is quite complicated and needs to be carried out before bliting the
  // object and the mask. Usually copy over to an intermediate buffer with one
  // extra horizontal column, and use it for the blit.

  // I think it could be done more easily than it seems though

  // at 2 pixels rotation - we have only 3 cases 2, 4, 6
  // in MSX rows are not aligned so is more expensive to do this
  // I think it would make sense to precompute the rotations actually
  // as we would have plenty of ROM space for doing this, and the outcome
  // would be a considerable speed up.

  // upon calculating the coordinates, we know which rotation is the right one
  //

  // it seems the original game doesn't do real occlusion
  // just calculates the order in which objects need to be drawn on every frame
  // I assume, but still is quite fast.

  // this experiments are quite promising I have to say, with precalculated tiles
  // we should be able to achieve a considerable speed up.

  //if (w == 4) {
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
    ld  b,#32     // 8 * 4 this can actually come from 7(ix) and shifted << 3
lb:
    exx
    ld  a,(bc)  ; read background -- this doesn't seem correct
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

    // TODO: optimize this
    ; this piece of code is extremely inefficient.. how to optimize.
    push hl
    push de
    ld h, b
    ld l, c
    ld de, #224  // 256 - 32 this depends on the viewport size ... 
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
//  }
}

/**
 * Blit tile object with mask
 */
void tileblit_object_show(TileObject *tileobject, uint8_t *buffer) __nonbanked
{


  // calculate target buffer position based on coordinates
  // copy over in width rows?

  TileSet *ts = tileobject->tileset;
  uint8_t *pattern = ts->pattern;
  uint8_t *color = ts->color;
  uint16_t size = ts->frame_w * ts->frame_h * 8;

  // this is totally wrong
  //

  // (y * 32)
  // this is fucking complicated


  // X - 0-->7 (offset= 0) X = 8-->15 (offset = 8)
  // Y - 1 (offset = 1) Y= 9 offset = (32 * 8) + 1

  uint16_t offset = (tileobject->x / 8) * 8 + (tileobject->y / 8) * 8 * 32 + tileobject->y % 8;


  // need to take into account the actual width
  //
  // sys has to be sys_memcpy_logic(dest, src1, src2, size, operation)
  // tileblit_mask()

  log_e("dest %x size %d\n", buffer+offset, offset);
//  sys_memcpy(buffer + offset, pattern, size);
  tileblit_object_mask(buffer+offset, pattern, 4, 4);
  //  tileblit_object_mask(buffer+offset+, pattern, 4, 4);

  // let's start with the simple case maybe?

  // problem is that we have 8x1 blocks and pixel coordinates
  // so non-byte X locations are problematic.

  // how the algorithm looks like?
  // - I would say we just blit everything to keep it simple and fast?
  // - pre-calculate X shifts ?

  // need to be able to shift a 32 bit value and mask and blit it at once
  // problem is that upon movement it become2 40 bit,
  // it can be done with RR chaining 5 registers
  //         SRA     B
  //      RR      C
  //       RR      D
  //      RR      E

  // so if X coordinate is % 8 = 0 just memcpy with mask
  // otherwise we take 1 extra byte per line, and rotate with mask % 8
  // then memcpu with mask, we do this per each line.

  // for static objects we can force the % 8 condition so that is faster
  // for dynamic objects, meh, we just need to do the calculation

  // in that sense there must be some limit for the size of the copy
  // which will be the amount of bytes we can effectively rotate before
  // doing the copy?

  // also, is there a way to do this operation _while_ memcpying? so that
  // is faster?

  // read target bytes
  // rotate source + mask and apply OR to target
  // this is going to be a complicated algorithm but doable.

  // no need to do flipping or anything, we have memory plenty
  // rotation is worth doing computationally, although it limits the size
  // of the dynamic objects... maybe we can have some special function
  // if we want to copy over big objects or something... I don't think
  // we need to do more than 64px wide in any case

}
