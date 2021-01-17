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

#ifdef MSX2

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
 * :param  page: the VRAM page to be used (0 to 3 MODE_GRP4, MODE_GRP5, 0 to 1 for MODE_GRP6)
 * :param  xpos: x coordinate of the location inside the page (0 to 255 MODE_GRP4),
 *       (0 to 511 in MODE_GRP5 and MODE_GRP6)
 */
void blit_set_to_vram(BlitSet *blitset, uint8_t page, uint16_t xpos, uint16_t ypos)
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
  for (i = 0; i < blitset->h; i ++) {
    vdp_memcpy(offset, src, blitset->w / ppb);
    offset += scr_w / ppb;
    src += blitset->w / ppb;
  }

  blitset->allocated = true;
  blitset->page = page;
  blitset->xpos = xpos;
  blitset->ypos = ypos;
}

void blit_object_show(BlitObject *bo) __nonbanked
{
  VdpCommand cmd;
  uint16_t frame_offset;

  frame_offset = (bo->state * bo->blitset->frames + bo->frame) * bo->blitset->frame_w;

  cmd.sx = bo->blitset->xpos + frame_offset;
  cmd.sy = bo->blitset->ypos + bo->blitset->page * 256;
  cmd.dx = bo->x;
  cmd.dy = bo->y;
  cmd.nx = bo->blitset->frame_w;
  cmd.ny = bo->blitset->frame_h;
  cmd.destdir = 0;
  cmd.command = (LMMM << 4) | TIMP;

  // blit_queue_cmd()
  vdp_exec(&cmd);
}

void blit_object_hide(BlitObject *blitobject) __nonbanked
{
  unused(blitobject);
}

#endif /* MSX2 */
