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

#include "msx.h"
#include "tile.h"
#include "font.h"
#include "vdp.h"
#include "log.h"
#include <stdio.h>

#pragma CODE_PAGE 2

#define BANK1_OFFSET 256 * 8
#define BANK2_OFFSET BANK1_OFFSET * 2

void init_font(struct font *f, uint8_t *tile_pattern, uint8_t *tile_color, uint8_t tile_w,
	uint8_t tile_h, enum font_type type, uint8_t num_glyphs, uint8_t glyph_w, uint8_t glyph_h)
{
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
void font_to_vram(struct font *f, uint8_t pos)
{
	if (&f->tiles != NULL) {
		tile_set_to_vram(&f->tiles, pos);
		f->idx = f->tiles.pidx;
	}
}

/**
 * Uploads a font to vram
 */
void font_to_vram_bank(struct font *f, uint8_t bank, uint8_t pos)
{
	if (&f->tiles != NULL) {
		tile_set_to_vram_bank(&f->tiles, bank, pos);
		f->idx = f->tiles.pidx;
	}
}

/**
 * Applies a color mask to a font already allocated in vram
 */
void font_color_mask(struct font *f, uint8_t color)
{
	uint16_t offset, size;
	struct tile_set *ts;

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
void font_set_color_mask(struct font_set *fs, uint8_t color)
{
	font_color_mask(fs->upper, color);
	font_color_mask(fs->lower, color);
	font_color_mask(fs->numeric, color);
	font_color_mask(fs->symbols, color);
}


static uint8_t font_emit_glyph(struct font *f, uint8_t *addr, char tc)
{
	uint8_t c, base_tile;
	c = tc * f->glyph_w;
	base_tile = f->tiles.pidx + c;

	if (f->glyph_w == 1 && f->glyph_h == 1) {
		*addr++ = base_tile;
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

/**
 * print a complex string to a buffer using a font_set
 *
 */
void font_printf(struct font_set *fs, uint8_t x, uint8_t y, uint8_t *buffer, char *text)
{
	char c, tc, base;
	uint8_t *addr = buffer + y * 32 + x;

	while ((c = *text++ ) != 0) {
		if (c == CHR_SPC) {
			addr++;
			continue;
		} else if (c >= CHR_0 && c <= CHR_9) {
			tc = c - CHR_0;
			addr += font_emit_glyph(fs->numeric, addr, tc);
		} else if (c >= CHR_A  && c <= CHR_Z) {
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
 * Prints a string directly to vram
 */
 // it should be possible to print to screen buffer instead of vdp
void font_vprint(struct font *f, uint8_t x, uint8_t y, char *text)
{
	char c, tc, base;
	uint16_t addr = vdp_base_names_grp1 + y * 32 + x;

	while ((c = *text++ ) != 0) {

		if (c == 32) {
			addr++;
		 	continue;
		}

		if (f->type == FONT_UPPERCASE) {
			if (c > 64 && c < 91) {
				tc = (c - 65) * f->glyph_w;
				base = f->tiles.pidx;
				if (f->glyph_w == 1 && f->glyph_h == 1) {
					vdp_poke(addr++, base + tc);
				} else if (f->glyph_w == 1 && f->glyph_h == 2) {
					vdp_poke(addr, base + tc);
					vdp_poke(addr + 32, base + f->tiles.w + tc);
					addr++;
				} else if (f->glyph_w == 2 && f->glyph_h == 2) {
					if (tc > 32) // support two lines
						tc += 32;
					vdp_poke(addr, base + tc);
					vdp_poke(addr + 1, base + tc + 1);
					vdp_poke(addr + 32, base + tc + f->tiles.w);
					vdp_poke(addr + 33, base + tc + f->tiles.w + 1);
					addr+=2;
				}
			}
		} else if (f->type == FONT_LOWERCASE) {
			if (c > 96 && c < 123) {
				tc = c - 97;
				base = f->tiles.pidx;
				if (f->glyph_w == 1 && f->glyph_h == 1) {
					vdp_poke(addr++, base + tc);
				} else if (f->glyph_w == 1 && f->glyph_h == 2) {
					vdp_poke(addr, base + tc);
					vdp_poke(addr + 32, base + f->tiles.w + tc);
					addr++;
				}
			}
		} else if (f->type == FONT_NUMERIC) {
			if (c > 47 && c < 58) {
				tc = c - 48;
				base = f->tiles.pidx;
				if (f->glyph_w == 1 && f->glyph_h == 2) {
					vdp_poke(addr, base + tc);
					vdp_poke(addr + 32, base + f->tiles.w + tc);
					addr++;
				}
			}
		}
	}

}
