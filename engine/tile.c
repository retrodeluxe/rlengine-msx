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

#define BANK1_OFFSET 256 * 8
#define BANK2_OFFSET BANK1_OFFSET * 2

#define RLE_BUFSIZE 255

#define BITMAP_TILEBANK_SIZE 32			/* 32 * 8 tiles */

uint8_t bitmap_tile_bank[BITMAP_TILEBANK_SIZE];
uint8_t inflate_buffer[RLE_BUFSIZE];

void tile_init()
{
	/* initialize bitmap to all ones : free */
	sys_memset(bitmap_tile_bank, 255, BITMAP_TILEBANK_SIZE);

	// first tile is reserved
	bitmap_reset(bitmap_tile_bank, 0);
}

static void tile_flush_inflate_buffer(uint16_t vram_offset, uint16_t size, uint8_t bank)
{
	if (bank == BANK0 || bank == ALLBANKS)
		vdp_copy_to_vram_di(inflate_buffer, vram_offset, size);
	if (bank == BANK1 || bank == ALLBANKS)
		vdp_copy_to_vram_di(inflate_buffer, vram_offset + BANK1_OFFSET, size);
	if (bank == BANK2 || bank == ALLBANKS)
		vdp_copy_to_vram_di(inflate_buffer, vram_offset + BANK2_OFFSET, size);
}

/**
 * Expands (RLE) a buffer all three char banks at the same time
 */
static void tile_inflate_to_vram(uint8_t *buffer, uint16_t offset, uint16_t size, uint8_t bank)
{
	int16_t count = size;
	uint8_t buffer_cnt = 0;
	uint16_t offset_vram = offset;
	uint8_t run_size;
	uint8_t *in = buffer;
	int16_t prev_byte = -1;
	uint8_t curr_byte;

	while (count > 0) {
		curr_byte = *in++;
		inflate_buffer[buffer_cnt++] = curr_byte;
		offset_vram++;
		count--;
		if (buffer_cnt == RLE_BUFSIZE) {
			tile_flush_inflate_buffer(offset_vram - RLE_BUFSIZE, RLE_BUFSIZE, bank);
			buffer_cnt = 0;
		}
		if (curr_byte == prev_byte) {
			run_size = *in++;
			while (run_size > 0 && count > 0) {
				inflate_buffer[buffer_cnt++] = curr_byte;
				count--;
				offset_vram++;
				run_size--;
				if (buffer_cnt == RLE_BUFSIZE) {
					tile_flush_inflate_buffer(offset_vram - RLE_BUFSIZE, RLE_BUFSIZE, bank);
					buffer_cnt = 0;
				}
			}
			prev_byte = -1;
		} else {
			prev_byte = curr_byte;
		}
	}
	tile_flush_inflate_buffer(offset_vram - buffer_cnt, buffer_cnt, bank);
}

/**
 * set a tileset in a fixed position.
 */
void tile_set_to_vram_bank(struct tile_set *ts, uint8_t bank, uint8_t pos)
{
	uint16_t size, offset, i;
	offset = pos * 8;
	size = ts->w * ts->h * 8;
	tile_inflate_to_vram(ts->pattern, vdp_base_chars_grp1 + offset, size, bank);
	tile_inflate_to_vram(ts->color, vdp_base_color_grp1 + offset, size, bank);
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
bool tile_set_valloc(struct tile_set *ts)
{
	uint16_t offset;
	uint8_t i, pos, size;
	bool found;

	if (ts->allocated) {
		return true;
	}

	size = ts->w * ts->h;

	found = bitmap_find_gap(bitmap_tile_bank, size, BITMAP_TILEBANK_SIZE - 1, &pos);
	if (!found) {
		sys_ascii_restore();
		bitmap_dump(bitmap_tile_bank, BITMAP_TILEBANK_SIZE -1);
		return false;
	}

	for (i = pos; i < pos + size; i++)
		bitmap_reset(bitmap_tile_bank, i);

	offset = pos * 8;
	tile_inflate_to_vram(ts->pattern, vdp_base_chars_grp1 + offset, size * 8, ALLBANKS);
	tile_inflate_to_vram(ts->color, vdp_base_color_grp1 + offset, size * 8, ALLBANKS);

	ts->allocated = true;
	ts->pidx = pos;
	return true;
}

/**
 * Force a tileset into a certain position
 *   this is useful when we have a map whose tiles are static and depend
 *   on the tiles to be in a specific location.
 */
void tile_set_to_vram(struct tile_set *ts, uint8_t pos)
{
	tile_set_to_vram_bank(ts, ALLBANKS, pos);
}

void tile_set_vfree(struct tile_set *ts)
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
void tile_object_show(struct tile_object *to, uint8_t * scrbuf, bool refresh_vram)
{
	uint16_t offset = to->x/8 + to->y/8 * 32;
	uint8_t *ptr = scrbuf + offset;
	uint8_t tile = to->ts->pidx + to->idx;
	uint8_t x,y;

	// FIXME: this doesn't seem to be correct
	if (to->ts->n_frames > 1)
		tile += to->ts->frame_w * to->cur_anim_step * to->cur_dir;

	for (y = 0; y < to->ts->frame_h; y++) {
		for (x = 0; x < to->ts->frame_w; x++) {
			*(ptr + x) = tile;
			if (refresh_vram) {
				vdp_poke(vdp_base_names_grp1 + offset + x, tile);
			}
			tile++;
		}
		ptr += 32;
		offset += 32;
		tile+= to->ts->w - to->ts->frame_w;
	}
	//log_e("showing : %d at pos %d dir %d step %d nf %d\n", tile, ptr, to->cur_dir, to->cur_anim_step, to->ts->n_frames);
}

void tile_object_hide(struct tile_object *to, uint8_t * scrbuf, bool refresh_vram)
{
	uint16_t offset = to->x/8 + to->y/8 * 32;
	uint8_t *ptr = scrbuf + offset;
	uint8_t x,y;

	for (y = 0; y < to->ts->frame_h; y++) {
		for (x = 0; x < to->ts->frame_w; x++) {
			*(ptr + x) = 0;
			if (refresh_vram) {
				vdp_poke(vdp_base_names_grp1 + offset + x, 0);
			}
		}
		ptr += 32;
		offset += 32;
	}
}
