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
#include "vdp.h"
#include "sys.h"
#include "tile.h"
#include "log.h"
#include "bitmap.h"

#pragma CODE_PAGE 2

#define BANK1_OFFSET 256 * 8
#define BANK2_OFFSET BANK1_OFFSET * 2

#define RLE_BUFSIZE 255

#define BITMAP_TILEBANK_SIZE 32			/* 32 * 8 tiles */

uint8_t bitmap_tile_bank[BITMAP_TILEBANK_SIZE];

void tile_init()
{
	/* initialize bitmap to all ones : free */
	sys_memset(bitmap_tile_bank, 255, BITMAP_TILEBANK_SIZE);

	// first tile is reserved
	bitmap_reset(bitmap_tile_bank, 0);
}

/**
 * set a tileset in a fixed position.
 */
void tile_set_to_vram_bank(TileSet *ts, uint8_t bank, uint8_t pos)
{
	uint16_t size, offset, i;
	offset = pos * 8;
	size = ts->w * ts->h * 8;
	if (bank == BANK0 || bank == ALLBANKS) {
		vdp_rle_inflate(vdp_base_chars_grp1 + offset, ts->pattern, size);
		vdp_rle_inflate(vdp_base_color_grp1 + offset, ts->color, size);
	}
	if (bank == BANK1 || bank == ALLBANKS) {
		vdp_rle_inflate(vdp_base_chars_grp1 + offset + BANK1_OFFSET, ts->pattern, size);
		vdp_rle_inflate(vdp_base_color_grp1 + offset + BANK1_OFFSET, ts->color, size);
	}
	if (bank == BANK2 || bank == ALLBANKS) {
		vdp_rle_inflate(vdp_base_chars_grp1 + offset + BANK2_OFFSET, ts->pattern, size);
		vdp_rle_inflate(vdp_base_color_grp1 + offset + BANK2_OFFSET, ts->color, size);
	}
	for (i = pos; i < pos + (size / 8); i++)
		bitmap_reset(bitmap_tile_bank, i);
	ts->allocated = true;
	ts->pidx = pos;
}

void tile_set_to_vram_bank_raw(TileSet *ts, uint8_t bank, uint8_t pos)
{
	uint16_t size, offset, i;
	offset = pos * 8;
	size = ts->w * ts->h * 8;
	if (bank == BANK0 || bank == ALLBANKS) {
		vdp_memcpy(vdp_base_chars_grp1 + offset, ts->pattern, size);
		vdp_memcpy(vdp_base_color_grp1 + offset, ts->color, size);
	}
	if (bank == BANK1 || bank == ALLBANKS) {
		vdp_memcpy(vdp_base_chars_grp1 + offset + BANK1_OFFSET, ts->pattern, size);
		vdp_memcpy(vdp_base_color_grp1 + offset + BANK1_OFFSET, ts->color, size);
	}
	if (bank == BANK2 || bank == ALLBANKS) {
		vdp_memcpy(vdp_base_chars_grp1 + offset + BANK2_OFFSET, ts->pattern, size);
		vdp_memcpy(vdp_base_color_grp1 + offset + BANK2_OFFSET, ts->color, size);
	}

	for (i = pos; i < pos + (size / 8); i++)
		bitmap_reset(bitmap_tile_bank, i);
	ts->allocated = true;
	ts->pidx = pos;
}

/**
 * tile_set_valloc
 *   attemps to allocate vram for a tileset
 * params:
 * return: true if success, false if failure
 */
bool tile_set_valloc(TileSet *ts)
{
	uint16_t offset, vsize;
	uint8_t i, pos, size;
	bool found;

	if (ts->allocated) {
		return true;
	}

	size = ts->w * ts->h;

	found = bitmap_find_gap(bitmap_tile_bank, size, BITMAP_TILEBANK_SIZE - 1, &pos);
	if (!found) {
		//ascii8_restore();
		//bitmap_dump(bitmap_tile_bank, BITMAP_TILEBANK_SIZE -1);
		return false;
	}

	for (i = pos; i < pos + size; i++)
		bitmap_reset(bitmap_tile_bank, i);

	offset = pos * 8;
	vsize = size * 8;
	if (ts->raw) {
		vdp_memcpy(vdp_base_chars_grp1 + offset, ts->pattern, vsize);
		vdp_memcpy(vdp_base_color_grp1 + offset, ts->color, vsize);
		vdp_memcpy(vdp_base_chars_grp1 + offset + BANK1_OFFSET, ts->pattern, vsize);
		vdp_memcpy(vdp_base_color_grp1 + offset + BANK1_OFFSET, ts->color, vsize);
		vdp_memcpy(vdp_base_chars_grp1 + offset + BANK2_OFFSET, ts->pattern, vsize);
		vdp_memcpy(vdp_base_color_grp1 + offset + BANK2_OFFSET, ts->color, vsize);
	} else {
		vdp_rle_inflate(vdp_base_chars_grp1 + offset, ts->pattern, vsize);
		vdp_rle_inflate(vdp_base_color_grp1 + offset, ts->color, vsize);
		vdp_rle_inflate(vdp_base_chars_grp1 + offset + BANK1_OFFSET, ts->pattern, vsize);
		vdp_rle_inflate(vdp_base_color_grp1 + offset + BANK1_OFFSET, ts->color, vsize);
		vdp_rle_inflate(vdp_base_chars_grp1 + offset + BANK2_OFFSET, ts->pattern, vsize);
		vdp_rle_inflate(vdp_base_color_grp1 + offset + BANK2_OFFSET, ts->color, vsize);
	}
	ts->allocated = true;
	ts->pidx = pos;
	return true;
}

/**
 * Force a tileset into a certain position
 *   this is useful when we have a map whose tiles are static and depend
 *   on the tiles to be in a specific location.
 */
void tile_set_to_vram(TileSet *ts, uint8_t pos)
{
	if (ts->allocated)
		return;

	if(ts->raw)
		tile_set_to_vram_bank_raw(ts, ALLBANKS, pos);
	else
		tile_set_to_vram_bank(ts, ALLBANKS, pos);
}

void tile_set_to_vram_raw(TileSet *ts, uint8_t pos)
{
	if (ts->allocated)
		return;

	tile_set_to_vram_bank_raw(ts, ALLBANKS, pos);
}

void tile_set_vfree(TileSet *ts)
{
	uint8_t i, size;

	if (!ts->allocated)
		return;
	size = ts->w * ts->h;
	for (i = ts->pidx; i < ts->pidx + size; i++)
		bitmap_set(bitmap_tile_bank, i);
	ts->allocated = false;
	ts->pidx = 0;
}

/**
 * puts on a screen buffer a set of tiles using an already allocated tileset
 */
void tile_object_show(TileObject *to, uint8_t * scrbuf, bool refresh_vram) __nonbanked
{
	uint16_t offset = to->x/8 + to->y/8 * 32;
	uint8_t *ptr = scrbuf + offset;
	uint8_t tile_base = to->ts->pidx;
	uint8_t tile = to->ts->pidx + to->idx;
	uint8_t x,y;

	tile += (to->ts->frame_w * to->cur_anim_step)
		+ to->cur_dir * (to->ts->frame_w * to->ts->n_frames);

	for (y = 0; y < to->ts->frame_h; y++) {
		for (x = 0; x < to->ts->frame_w; x++) {
			*(ptr + x) = tile;
			if (refresh_vram) {
				vdp_write(vdp_base_names_grp1 + offset + x, tile);
			}
			tile++;
			/** allow for shift using idx: this will only work
			    for single frame one direction objects **/
			if (to->idx > 0 && tile > tile_base + to->ts->frame_w -1)
				tile = tile_base;
		}
		ptr += 32;
		offset += 32;
		tile+= to->ts->w - to->ts->frame_w;
	}
	//log_e("showing : %d at pos %d dir %d step %d nf %d\n", tile, ptr, to->cur_dir, to->cur_anim_step, to->ts->n_frames);
}

void tile_object_hide(TileObject *to, uint8_t * scrbuf, bool refresh_vram) __nonbanked
{
	uint16_t offset = to->x/8 + to->y/8 * 32;
	uint8_t *ptr = scrbuf + offset;
	uint8_t x,y;

	for (y = 0; y < to->ts->frame_h; y++) {
		for (x = 0; x < to->ts->frame_w; x++) {
			*(ptr + x) = 0;
			if (refresh_vram) {
				vdp_write(vdp_base_names_grp1 + offset + x, 0);
			}
		}
		ptr += 32;
		offset += 32;
	}
}
