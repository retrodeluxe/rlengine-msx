/*
 * RetroDeLuxe Engine MSX1
 *
 * Copyright (C) 2013 Enric Geijo
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

 /**
 * read a buffer an expand it using a 4x4 tile dictionary
 *  this provides a fixed ratio of 1/4 and decompression buffer
 *  size is fixed.
 *
 *   width of the map is needed
 *   out buffer needs to be 4 times data_size
 */
void map_inflate(const byte * dict, const byte * in, byte * out, uint16_t data_size,
		 byte w)
{
	byte col = 0;
	unsigned int idx;
	byte *src;
	byte *dst = out;
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
void map_inflate_screen(const byte * dict, const byte * in, byte * out, byte w, byte vpx, byte vpy)
{
	unsigned int idx;
	byte *base = in + vpx / 2 + (vpy / 2) * w;
	byte *src = base;
	byte *dst = out;
	int x,y;

	for (y = 0; y < 24; y+=2) {
		for (x = 0; x < 32; x+=2) {
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