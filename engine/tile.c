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
#include "vdp.h"
#include "sys.h"
#include "tile.h"
#include "log.h"
#include "bitmap.h"

uint8_t bitmap_tile_bank[32];

void tile_init()
{
	/* initialize bitmap to all ones : free */
	sys_memset(bitmap_tile_bank, 255, 32);

	// first tile is reserved
	bitmap_reset(bitmap_tile_bank, 0);
}

/**
 * set a tileset in a fixed position.
 */
void tile_set_to_vram_bank(struct tile_set *ts, uint8_t bank, uint8_t pos)
{
	uint16_t size, offset, i;
	offset = 256 * 8 * bank + pos * 8;
	size = ts->w * ts->h * 8;
	vdp_copy_to_vram(ts->pattern, vdp_base_chars_grp1 + offset, size);
	vdp_copy_to_vram(ts->color, vdp_base_color_grp1 + offset, size);
	for (i = pos; i < pos + (size / 8); i++)
		bitmap_reset(bitmap_tile_bank, i);
	ts->allocated = true;
	ts->pidx = pos;
}

/**
 * alloc a tileset in all banks
 */
void tile_set_valloc(struct tile_set *ts)
{
	uint16_t offset;
	uint8_t i, pos, size;

	if (ts->allocated)
		return;

	size = ts->w * ts->h;
	pos = bitmap_find_gap(bitmap_tile_bank, size, 31);

	// TODO: check for fail
	for (i = pos; i < pos + size; i++)
		bitmap_reset(bitmap_tile_bank, i);

	for (i = 0; i < 3; i++) {
		offset = 256 * 8 * i + pos * 8;
		vdp_copy_to_vram(ts->pattern, vdp_base_chars_grp1 + offset, size * 8);
		vdp_copy_to_vram(ts->color, vdp_base_color_grp1 + offset, size * 8);
	}

	ts->allocated = true;
	ts->pidx = pos;
	log_e("allocated tile set at pos %d\n", pos);
}

/**
 * Force a tileset into a certain position
 *   this is useful when we have a map whose tiles are static and depend
 *   on the tiles to be in a specific location.
 */
void tile_set_to_vram(struct tile_set *ts, uint8_t pos)
{
	tile_set_to_vram_bank(ts, 0, pos);
	tile_set_to_vram_bank(ts, 1, pos);
	tile_set_to_vram_bank(ts, 2, pos);
}

// FIMXE: redefie viewport and map pos...
// void tile_map_clip(struct tile_map *tm,
// 		      struct gfx_viewport *vp, uint8_t * scrbuf,
// 		      struct gfx_map_pos *p)
// {
// 	uint8_t i;
// 	uint8_t *ptr = scrbuf + vp->x + vp->y * gfx_screen_tile_w;
// 	uint8_t *src = tm->map + p->x + p->y * tm->w;
//
// 	for (i = 0; i <= vp->h; i++) {
// 		sys_memcpy(ptr, src, vp->w);
// 		ptr += gfx_screen_tile_w;
// 		src += tm->w;
// 	}
// }

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
