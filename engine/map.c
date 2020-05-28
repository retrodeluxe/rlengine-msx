/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#include "map.h"

#pragma CODE_PAGE 2

 /**
 * read a buffer an expand it using a 4x4 tile dictionary
 *  this provides a fixed ratio of 1/4 and decompression buffer
 *  size is fixed.
 *
 *   width of the map is needed
 *   out buffer needs to be 4 times data_size
 */
void map_inflate(const uint8_t * dict, uint8_t * in, uint8_t * out, uint16_t data_size,
		 uint8_t w)
{
	uint8_t col = 0;
	unsigned int idx;
	uint8_t *src;
	uint8_t *dst = out;
	/* FIXME: optimize this */
	for (src = in; src < in + data_size; src++) {
		idx = (*src) * 4;
		*(dst) = dict[idx];
		*(dst + 1) = dict[idx + 1];
		*(dst + w) = dict[idx + 2];
		*(dst + w + 1) = dict[idx + 3];
		col += 2;
		dst += 2;
		if (col >= w) {
			col = 0;
			dst += w;
		}
	}
}

/**
 * Inflate only a 32x24 window of the map to a buffer, useful if the map is too big
 * to be decompressed entirely.
 */
void __map_inflate_screen(const uint8_t * dict, uint16_t * in, uint8_t * out, uint8_t w, uint8_t vpx, uint8_t vpy)
{
	unsigned int idx;
	uint16_t *base = in + vpx / 2 + (vpy / 2) * w / 2;
	uint16_t *src = base;
	uint8_t *dst = out;
	int x,y;

	for (y = 0; y < 24; y += 2) {
		for (x = 0; x < 32; x += 2) {
			idx = (*src) * 4;
			*(dst) = dict[idx];
			*(dst + 1) = dict[idx + 1];
			*(dst + 32) = dict[idx + 2];
			*(dst + 32 + 1) = dict[idx + 3];
			dst = out + x + y * 32;
			src = base + x / 2 + y / 2 * w / 2;
		}
	}
}
