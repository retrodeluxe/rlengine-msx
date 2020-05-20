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
 * Prints a string directly to vram
 */
void font_vprint(struct font *f, uint8_t x, uint8_t y, char *text)
{
	char c, tc, base;
	uint16_t addr = vdp_base_names_grp1 + y * 32 + x;

	while ((c = *text++ ) != '~'') {

		if (c == 32) {
			addr++;
		 	continue;
		}

		if (f->type == FONT_ALFA_UPPERCASE) {
			if (c > 64 && c < 91) {
				tc = c - 65;
				base = f->tiles.pidx;
				if (f->glyph_w == 1 && f->glyph_h == 1)
					vdp_poke(addr++, base + tc);
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
