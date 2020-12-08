/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2019 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#include "font.h"
#include "log.h"
#include "msx.h"
#include "tile.h"
#include "vdp.h"
#include <stdio.h>

#pragma CODE_PAGE 2

#define BANK1_OFFSET 256 * 8
#define BANK2_OFFSET BANK1_OFFSET * 2

void init_font(Font *f, uint8_t *tile_pattern, uint8_t *tile_color,
               uint8_t tile_w, uint8_t tile_h, FontType type,
               uint8_t num_glyphs, uint8_t glyph_w, uint8_t glyph_h) {
  f->tiles.w = tile_w;
  f->tiles.h = tile_h;
  f->tiles.pattern = tile_pattern;
  f->tiles.color = tile_color;
  f->tiles.allocated = false;
  f->type = type;
  f->num_glyphs = num_glyphs;
  f->glyph_h = glyph_h;
  f->glyph_w = glyph_w;
}
/**
 *
 */
void font_to_vram(Font *f, uint8_t pos) {
  if (&f->tiles != NULL) {
    tile_set_to_vram_raw(&f->tiles, pos);
    f->idx = f->tiles.pidx;
  }
}

/**
 * Uploads a font to vram
 */
void font_to_vram_bank(Font *f, uint8_t bank, uint8_t pos) {
  if (&f->tiles != NULL) {
    tile_set_to_vram_bank_raw(&f->tiles, bank, pos);
    f->idx = f->tiles.pidx;
  }
}

void font_vfree(Font *f) {
  if (f != NULL) {
    tile_set_vfree(&f->tiles);
  }
}

void font_set_vfree(FontSet *fs) {
  if (fs->upper != NULL)
    tile_set_vfree(&fs->upper->tiles);
  if (fs->lower != NULL)
    tile_set_vfree(&fs->lower->tiles);
  if (fs->numeric != NULL)
    tile_set_vfree(&fs->numeric->tiles);
  if (fs->symbols != NULL)
    tile_set_vfree(&fs->symbols->tiles);
}

/**
 * Applies a color mask to a font already allocated in vram
 */
void font_color_mask(Font *f, uint8_t color) {
  uint16_t offset, size;
  TileSet *ts;

  ts = &f->tiles;
  if (ts->allocated) {
    offset = ts->pidx * 8;
    size = ts->w * ts->h * 8;
    vdp_memset(vdp_base_color_grp1 + offset, size, color);
    vdp_memset(vdp_base_color_grp1 + offset + BANK1_OFFSET, size, color);
    vdp_memset(vdp_base_color_grp1 + offset + BANK2_OFFSET, size, color);
  }
}

/**
 * Apply color mask to a font set already allocated in vram
 */
void font_set_color_mask(FontSet *fs, uint8_t color) {
  if (fs->upper != NULL)
    font_color_mask(fs->upper, color);
  if (fs->lower != NULL)
    font_color_mask(fs->lower, color);
  if (fs->numeric != NULL)
    font_color_mask(fs->numeric, color);
  if (fs->symbols != NULL)
    font_color_mask(fs->symbols, color);
}

static uint8_t font_emit_glyph(Font *f, uint8_t *addr, char tc) {
  uint8_t c, base_tile;
  c = tc * f->glyph_w;
  base_tile = f->tiles.pidx + c;

  if (f->glyph_w == 1 && f->glyph_h == 1) {
    *addr = base_tile;
    return 1;
  } else if (f->glyph_w == 1 && f->glyph_h == 2) {
    *addr = base_tile;
    *(addr + 32) = base_tile + f->tiles.w;
    return 1;
  } else if (f->glyph_w == 2 && f->glyph_h == 2) {
    if (c > 32) // wide glyphs may overflow in two rows
      base_tile += 32;
    *addr = base_tile;
    *(addr + 1) = base_tile + 1;
    *(addr + 32) = base_tile + f->tiles.w;
    *(addr + 33) = base_tile + f->tiles.w + 1;
    return 2;
  }
  return 1;
}

static uint8_t font_emit_glyph_vram(Font *f, uint16_t addr, char tc) {
  uint8_t c, base_tile;
  c = tc * f->glyph_w;
  base_tile = f->tiles.pidx + c;

  if (f->glyph_w == 1 && f->glyph_h == 1) {
    vdp_write(addr, base_tile);
    return 1;
  } else if (f->glyph_w == 1 && f->glyph_h == 2) {
    vdp_write(addr, base_tile);
    vdp_write(addr + 32, base_tile + f->tiles.w);
    return 1;
  } else if (f->glyph_w == 2 && f->glyph_h == 2) {
    if (c > 32) // wide glyphs may overflow in two rows
      base_tile += 32;
    vdp_write(addr, base_tile);
    vdp_write(addr + 1, base_tile + 1);
    vdp_write(addr + 32, base_tile + f->tiles.w);
    vdp_write(addr + 33, base_tile + f->tiles.w + 1);
    return 2;
  }
  return 1;
}

/**
 * print a complex string to a buffer using a font_set
 *
 */
void font_printf(FontSet *fs, uint8_t x, uint8_t y, uint8_t *buffer,
                 char *text) {
  char c, tc, base;
  uint8_t *addr = buffer + y * 32 + x;

  while ((c = *text++) != 0) {
    if (c == CHR_SPC) {
      addr++;
      continue;
    } else if (c >= CHR_0 && c <= CHR_9) {
      tc = c - CHR_0;
      addr += font_emit_glyph(fs->numeric, addr, tc);
    } else if (c >= CHR_A && c <= CHR_Z) {
      tc = c - CHR_A;
      addr += font_emit_glyph(fs->upper, addr, tc);
    } else if (c >= CHR_a && c <= CHR_z) {
      tc = c - CHR_a;
      addr += font_emit_glyph(fs->lower, addr, tc);
    } else if (c >= CHR_EXCL && c <= CHR_SLASH) {
      tc = c - CHR_EXCL;
      addr += font_emit_glyph(fs->symbols, addr, tc);
    }
  }
}

/**
 * prints a complex string directly to vram
 */
void font_vprintf(FontSet *fs, uint8_t x, uint8_t y, char *text) {
  char c, tc, base;
  uint16_t addr = vdp_base_names_grp1 + y * 32 + x;

  while ((c = *text++) != 0) {
    if (c == CHR_SPC) {
      addr++;
      continue;
    } else if (c >= CHR_0 && c <= CHR_9) {
      tc = c - CHR_0;
      addr += font_emit_glyph_vram(fs->numeric, addr, tc);
    } else if (c >= CHR_A && c <= CHR_Z) {
      tc = c - CHR_A;
      addr += font_emit_glyph_vram(fs->upper, addr, tc);
    } else if (c >= CHR_a && c <= CHR_z) {
      tc = c - CHR_a;
      addr += font_emit_glyph_vram(fs->lower, addr, tc);
    } else if (c >= CHR_EXCL && c <= CHR_SLASH) {
      tc = c - CHR_EXCL;
      addr += font_emit_glyph_vram(fs->symbols, addr, tc);
    }
  }
}
