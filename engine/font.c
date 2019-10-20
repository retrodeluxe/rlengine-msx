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
/**
 *
 */
void font_to_vram(struct font *f, uint8_t pos)
{
	if (&f->upper != NULL) {
		tile_set_to_vram(&f->upper, pos);
		f->upper_idx = f->upper.pidx;
	}
}

/**
 * Uploads a font to vram
 */
void font_to_vram_bank(struct font *f, uint8_t bank, uint8_t pos)
{
	if (&f->upper != NULL) {
		tile_set_to_vram_bank(&f->upper, bank, pos);
		f->upper_idx = f->upper.pidx;
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
		// char translation based on font indexes
		if (c == 32) {
			addr++;
		 	continue;
		}
		// upper case font
		if (c > 64 && c < 91) {
			tc = c - 65;
			log_e("printing %d %d\n", base, tc);
			base = f->upper.pidx;
			vdp_poke(addr++, base + tc);
		}
	}

}
