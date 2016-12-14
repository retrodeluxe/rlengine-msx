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

void tile_set_to_vram_bank(struct tile_set *ts, uint8_t bank, uint8_t pos)
{
	uint16_t size, offset;
	offset = 256 * 8 * bank + pos * 8;
	size = ts->w * ts->h * 8;
	vdp_copy_to_vram(ts->pattern, vdp_base_chars_grp1 + offset, size);
	vdp_copy_to_vram(ts->color, vdp_base_color_grp1 + offset, size);
}

void tile_set_to_vram(struct tile_set *ts, uint8_t pos)
{
	tile_set_to_vram_bank(ts, 0, pos);
	tile_set_to_vram_bank(ts, 1, pos);
	tile_set_to_vram_bank(ts, 2, pos);
}

void tile_map_clip(struct tile_map *tm,
		      struct gfx_viewport *vp, uint8_t * scrbuf,
		      struct gfx_map_pos *p)
{
	uint8_t i;
	uint8_t *ptr = scrbuf + vp->x + vp->y * gfx_screen_tile_w;
	uint8_t *src = tm->map + p->x + p->y * tm->w;

	for (i = 0; i <= vp->h; i++) {
		sys_memcpy(ptr, src, vp->w);
		ptr += gfx_screen_tile_w;
		src += tm->w;
	}
}