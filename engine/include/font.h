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

#ifndef _MSX_FONT_H_
#define _MSX_FONT_H

#include "tile.h"

struct font {
	struct tile_set upper;
	struct tile_set lower;
	struct tile_set digit;
	uint8_t upper_idx;
	uint8_t lower_idx;
	uint8_t digit_idx;
};

#define INIT_FONT(FONT, UPPER)	(FONT).upper.w = UPPER ## _tile_w;\
				(FONT).upper.h = UPPER ## _tile_h;\
				(FONT).upper.pattern = UPPER ## _tile;\
				(FONT).upper.color = UPPER ## _tile_color; \
				(FONT).upper.allocated = false;

void font_to_vram(struct font *f, uint8_t pos);
void font_to_vram_bank(struct font *f, uint8_t bank, uint8_t pos);
void font_vprint(struct font *f, uint8_t x, uint8_t y, char *text);

#endif
