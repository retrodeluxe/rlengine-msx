/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2020 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#include "vdp.h"
#include "blit.h"
#include "log.h"
#include <stdio.h>

#ifdef MSX2

#pragma CODE_PAGE 2

static VdpCommand cmd;

void blit_init()
{
}

rle_result blit_set_valloc(BlitSet *blitset)
{
  unused(blitset);
  return RLE_OK;
}

void blit_set_vfree(BlitSet *blitset)
{
  unused(blitset);
}

/**
 * Manually allocate and copy a BlitSet into VRAM
 *
 * :param blitset: the BlitSet to be allocated
 * :param  page: the VRAM page to be used (0 to 3 MODE_GRP4, 0 to 1 for MODE_GRP6)
 * :param  xpos: x coordinate of the location inside the page (0 to 255 MODE_GRP4),
 *       (0 to 511 MODE GRP6)
 */
void blit_set_to_vram(BlitSet *blitset, uint8_t page, uint16_t xpos, uint16_t ypos) __nonbanked
{
  uint16_t offset, vram_page = 0, i, scr_w = 0, ppb = 0;
  uint8_t *src;

  uint8_t mode = vdp_get_mode();
  if(mode == MODE_GRP4) {
    vram_page = page * 2;
    scr_w = MODE_GRP4_SCR_WIDTH;
    ppb = MODE_GRP4_PIX_PER_BYTE;
  } else if (mode == MODE_GRP6) {
    vram_page = page * 4;
    scr_w = MODE_GRP6_SCR_WIDTH;
    ppb = MODE_GRP6_PIX_PER_BYTE;
  }

  vdp_set_vram_page(vram_page);

  src = blitset->bitmap;

  offset = xpos + ypos * scr_w / ppb;
  if (blitset->raw) {
    for (i = 0; i < blitset->h; i ++) {
      vdp_memcpy(offset, src, blitset->w / ppb);
      src += blitset->w / ppb;
      offset += scr_w / ppb;
    }
  } else {
      vdp_rle_inflate(offset, src, blitset->w * blitset->h / ppb);
  }

  blitset->allocated = true;
  blitset->page = page;
  blitset->xpos = xpos;
  blitset->ypos = ypos;
}

void blit_object_show(BlitObject *bo) __nonbanked
{
  uint16_t frame_offset;

  // copy background to mask
  cmd.sx = bo->prev_x;
  cmd.sy = bo->prev_y + 255;
  cmd.dx = bo->mask_x;
  cmd.dy = bo->mask_y + bo->mask_page * 256;
  cmd.nx = bo->blitset->frame_w + 4;
  cmd.ny = bo->blitset->frame_h + 4;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);
  vdp_exec(&cmd);

  // logic copy bitmap on top of mask
  frame_offset = (bo->state * bo->blitset->frames + bo->frame) * bo->blitset->frame_w;

  cmd.sx = bo->blitset->xpos + frame_offset;
  cmd.sy = bo->blitset->ypos + bo->blitset->page * 256;
  cmd.dx = bo->mask_x + 2;
  cmd.dy = bo->mask_y + 2 + bo->mask_page * 256;
  cmd.nx = bo->blitset->frame_w;
  cmd.ny = bo->blitset->frame_h;
  cmd.destdir = 0;
  cmd.command = (LMMM << 4) | TIMP;
  vdp_exec(&cmd);

  // composite object
  if (bo->blitset2 != NULL) {
    frame_offset = (bo->state2 * bo->blitset2->frames + bo->frame2) * bo->blitset2->frame_w;

    cmd.sx = bo->blitset2->xpos + frame_offset;
    cmd.sy = bo->blitset2->ypos + bo->blitset2->page * 256;
    cmd.dx = bo->mask_x + 2 + bo->offsetx2;
    cmd.dy = bo->mask_y + 2 + bo->offsety2 + bo->mask_page * 256;
    cmd.nx = bo->blitset2->frame_w;
    cmd.ny = bo->blitset2->frame_h;
    cmd.destdir = 0;
    cmd.command = (LMMM << 4) | TIMP;
    vdp_exec(&cmd);
  }

  // copy over set to page 0
  cmd.sx = bo->mask_x;
  cmd.sy = bo->mask_y + bo->mask_page * 256;
  cmd.dx = bo->prev_x;
  cmd.dy = bo->prev_y;
  cmd.nx = bo->blitset->frame_w + 4;
  cmd.ny = bo->blitset->frame_h + 4;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);
  vdp_exec(&cmd);

  bo->prev_x = bo->x;
  bo->prev_y = bo->y;
}

void blit_object_update(BlitObject *bo) __nonbanked
{
  blit_object_show(bo);
}

void blit_object_hide(BlitObject *bo) __nonbanked
{
  uint16_t frame_offset;

  // just copy over from page 1 to page 0
  cmd.sx = bo->prev_x;
  cmd.sy = bo->prev_y + 256;
  cmd.dx = bo->prev_x;
  cmd.dy = bo->prev_y;
  cmd.nx = bo->blitset->frame_w;
  cmd.ny = bo->blitset->frame_h;
  cmd.color = 0;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);

  vdp_exec(&cmd);
}

/**
 * Blits a screen tilebuffer of 32x24 tiles using the provided BlitSet
 *
 * :param buffer: tilebuffer
 * :param bs: the blitset to be used
 */
void blit_map_tilebuffer(uint8_t *buffer, BlitSet *bs, uint8_t page) __nonbanked
{
  uint16_t i, j, offset_x, offset_y;
  uint16_t dp = page * 256;

  cmd.nx = bs->frame_w;
  cmd.ny = bs->frame_h;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);
  for (j = 0; j < 16; j++) {
    cmd.dy = (j << 4) + dp;
    for (i = 0; i < 16; i++) {
      offset_x = ((*buffer -1) & 15) << 4;
      offset_y = ((*buffer -1) >> 4) << 4;
      cmd.sx = bs->xpos + offset_x;
      cmd.sy = bs->ypos + offset_y + bs->page * 256;
      cmd.dx = i << 4;
      vdp_exec(&cmd);
      buffer++;
    }
  }
}

void blit_map_tilebuffer_rect(uint8_t *buffer, BlitSet *bs, uint8_t page,
  uint8_t x, uint8_t y, uint8_t w, uint8_t h) __nonbanked
{
  uint16_t i, j, offset_x, offset_y;
  uint16_t dp = page * 256;

  cmd.nx = bs->frame_w;
  cmd.ny = bs->frame_h;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);
  for (j = y; j < y + h; j += 2) {
    cmd.dy = (j << 3) + dp; // maybe this should be 4?
    for (i = x; i < x + w; i += 2) {
      offset_x = ((*buffer -1) & 15) << 4;
      offset_y = ((*buffer -1) >> 4) << 4;
      cmd.sx = bs->xpos + offset_x;
      cmd.sy = bs->ypos + offset_y + bs->page * 256;
      cmd.dx = i << 3;
      vdp_exec(&cmd);
      buffer++;
    }
    //buffer+=16;
  }
}

void blit_font_vprintf(BlitSet *bs, uint8_t x, uint8_t y, uint8_t page, char *text) __nonbanked
{
  uint16_t dx, offset_x, offset_y;
  uint16_t dp = page * 256;
  char c;

  cmd.nx = bs->frame_w;
  cmd.ny = bs->frame_h;
  cmd.destdir = 0;
  cmd.command = (HMMM << 4);

  dx = 0;
  while ((c = *text++) != 0) {
    c = c - 32;
    cmd.dy = (y << 3) + dp;
    offset_x = (c & 15) << 4;
    offset_y = (c >> 4) << 4;
    cmd.sx = bs->xpos + offset_x;
    cmd.sy = bs->ypos + offset_y + bs->page * 256;
    cmd.dx = (x + dx) << 3;
    vdp_exec(&cmd);
    dx += 1;
  }
}

#endif /* MSX2 */
