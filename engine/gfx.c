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


/*
 * divide tile banks in quads of 2x2 tiles, and set three groups
 *
 *   - scenery tiles
 *	 - dynamic tiles
 *   - sprite tiles
 *
 * of 64 possible quads we assing 12 to dyntiles 16 to sprites and
 * group them in columns to ease collision detection.
 *
 * dynamic tiles are used for overlaying objects on top of the map
 * that are susceptible of removal or change
 *
 * sprite tiles are used for overlaying animated, full color tiles
 * on top of the map.
 *
 * allocation of dynamic and sprite tiles is "dynamic", we only upload
 * data to vram when needed, and garbage collection is necessary.
 *
 * the remaining quads (34) used for scenery graphics.
 *
 */
#define NUM_BANKS 3
#define NUM_DYN_QUADS 12
#define NUM_SPR_QUADS 16

#define QUAD_TO_TILE(_n) (((_n % 16) << 1) + ((_n / 16) << 6))
#define TILE_TO_QUAD(_n) (((_n % 32) >> 1) + ((_n / 64) << 4))

/* the list of actual quads need to go into a define actually */
static const byte dyntile_quad[NUM_DYN_QUADS] = { 13, 14, 15,
	29, 30, 31,
	45, 46, 47,
	61, 62, 63
};

static const byte sprite_quad[NUM_SPR_QUADS] = { 9, 10, 11, 12,
	25, 26, 27, 28,
	41, 42, 43, 44,
	57, 58, 59, 60
};

byte dyn_quad_list[NUM_BANKS][NUM_DYN_QUADS];
byte dyn_sprite_list[NUM_BANKS][NUM_SPR_QUADS];

void gfx_dyntile_init(void)
{
	/*
	 * Initialize quad lists by setting the values to 1,
	 * which means that all quads are free.
	 */
	sys_memset(dyn_quad_list, 1, sizeof(dyn_quad_list));
	sys_memset(dyn_sprite_list, 1, sizeof(dyn_sprite_list));
}

static void upload_quad(byte quad, byte bank, byte source_tile,
			struct gfx_tilebank *tb)
{
	uint offset_ptn_vram, offset_ptn_tilebank;
	uint offset_clr_vram, offset_clr_tilebank;
	uint offset_vram;

	offset_vram = 2048 * bank + QUAD_TO_TILE(quad) * 8;
	offset_ptn_vram = vdp_base_chars_grp1 + offset_vram;
	offset_ptn_tilebank = (uint) tb->pattern + 8 * source_tile;;
	offset_clr_vram = vdp_base_color_grp1 + offset_vram;
	offset_clr_tilebank = (uint) tb->color + 8 * source_tile;

	vdp_fastcopy16(offset_ptn_tilebank, offset_ptn_vram);
	vdp_fastcopy16(offset_clr_tilebank, offset_clr_vram);
	vdp_fastcopy16(offset_ptn_tilebank + 256, offset_ptn_vram + 256);
	vdp_fastcopy16(offset_clr_tilebank + 256, offset_clr_vram + 256);
}

static byte gfx_dyntile_set(struct gfx_tilemap_object *obj, byte bank,
			    struct gfx_tilebank *tilebank)
{
	byte last_free = 0xff;
	byte i;

	for (i = 0; i < NUM_DYN_QUADS; i++) {
		if (dyn_quad_list[bank][i] == 1) {
			last_free = i;
		} else if (dyn_quad_list[bank][i] == obj->tile) {
			/* tile exists */
			return QUAD_TO_TILE(dyntile_quad[i]);
		}
	}

	if (last_free == 0xff) {	/* we are full */
		LOGW("GFX\t WARNING dyntile bank [%d] full\n", bank);
		return 0;
	}

	upload_quad(dyntile_quad[last_free], bank, obj->tile, tilebank);
	dyn_quad_list[bank][last_free] = obj->tile;

	return QUAD_TO_TILE(dyntile_quad[last_free]);
}

/*
 * gfx_sprite_set
 *
 *  Check if there is a quad containing the set of tiles we need.
 *
 *  - If one is found return the tile id
 *  - If none is found, upload tile data to vram to the last free position.
 *  - If there are no free tiles to upload returns 0
 *
 */
static byte gfx_sprite_set(struct gfx_sprite_def *spr, byte bank,
			   struct gfx_tilebank *tilebank)
{
	byte last_free = 0xff;
	byte i;

	for (i = 0; i < NUM_SPR_QUADS; i++) {
		if (dyn_sprite_list[bank][i] == 1) {
			last_free = i;
		} else if (dyn_sprite_list[bank][i] == spr->tile) {
			/* tile exists */
			LOGD("spr tile %d exists\n", spr->tile);
			return QUAD_TO_TILE(sprite_quad[i]);
		}
	}

	if (last_free == 0xff) {	/* we are full */
		LOGW("GFX\t WARNING sprite bank [%d] full\n", bank);
		return 0;
	}

	upload_quad(sprite_quad[last_free], bank, spr->tile, tilebank);
	dyn_sprite_list[bank][last_free] = spr->tile;

	return QUAD_TO_TILE(sprite_quad[last_free]);
}

void gfx_dyntile_clear(struct gfx_tilemap_object *obj)
{
	byte i;
	/* coordinates are outside the screen, need to clear in all banks */
	for (i = 0; i < NUM_DYN_QUADS; i++) {
		if (dyn_quad_list[0][i] == obj->tile)
			dyn_quad_list[0][i] = 1;
		if (dyn_quad_list[1][i] == obj->tile)
			dyn_quad_list[1][i] = 1;
		if (dyn_quad_list[2][i] == obj->tile)
			dyn_quad_list[2][i] = 1;
	}
	//LOGD("GFX\t clearing dyntile [%d]\n",obj->tile);
}

void gfx_sprite_show(struct gfx_sprite_def *spr,
		     struct gfx_tilebank *tilebank, byte x, byte y,
		     byte * scrbuf)
{
	byte id, id2;
	byte vdp_bank = y / 8;
	byte vdp_bank_2 = 0;

	byte *ptr = scrbuf + x + y * gfx_screen_tile_w;

	if (spr->tile == 255)
		return;

	if (y == 7)
		vdp_bank_2 = 1;
	if (y == 15)
		vdp_bank_2 = 2;

	id = gfx_sprite_set(spr, vdp_bank, tilebank);

	id2 = id;
	if (vdp_bank_2)
		id2 = gfx_sprite_set(spr, vdp_bank_2, tilebank);

	/* full, do not draw */
	if (id == 0 || id2 == 0)
		return;

	*(ptr) = id;
	*(ptr + 1) = id + 1;
	*(ptr + gfx_screen_tile_w) = id2 + 32;
	*(ptr + gfx_screen_tile_w + 1) = id2 + 33;
}

void gfx_sprite_move(struct gfx_sprite_def *spr, byte dir, byte steps,
		     char collision)
{
	/* here, change the sprite tiles to produce anymation effect
	   also translation */

	/* this set the machine to the limit of the performance already, I think,
	   optimizations are going to be necessary... */

	/* gfx_sprite_show by changing the tile and the position,
	   and the tile must be the current tile, in a similiar way as it is done
	   with spr_calc_patterns... */

	/* also I think it would be good to upload all sprites at once */

	/* and thinking more of it, making a static allocation would made the code way faster,
	   but it will reduce the flexibility, if performance allows for it, I would go for
	   keeping it dynamic */

}

void gfx_sprite_clear(struct gfx_sprite_def *spr)
{
	byte i;

	for (i = 0; i < NUM_SPR_QUADS; i++) {
		if (dyn_sprite_list[0][i] == spr->tile)
			dyn_sprite_list[0][i] = 1;
		if (dyn_sprite_list[1][i] == spr->tile)
			dyn_sprite_list[1][i] = 1;
		if (dyn_sprite_list[2][i] == spr->tile)
			dyn_sprite_list[2][i] = 1;
	}
}

void gfx_dyntile_show(struct gfx_tilemap_object *obj,
		      struct gfx_tilebank *tilebank, byte x, byte y,
		      byte * scrbuf)
{
	byte id, id2;
	byte vdp_bank = y / 8;
	byte vdp_bank_2 = 0;

	byte *ptr = scrbuf + x + y * gfx_screen_tile_w;

	if (obj->tile == 255)
		return;

	if (y == 7)
		vdp_bank_2 = 1;
	if (y == 15)
		vdp_bank_2 = 2;

	id = gfx_dyntile_set(obj, vdp_bank, tilebank);

	id2 = id;
	if (vdp_bank_2)
		id2 = gfx_dyntile_set(obj, vdp_bank_2, tilebank);

	/* full, do not draw */
	if (id == 0 || id2 == 0)
		return;

	*(ptr) = id;
	*(ptr + 1) = id + 1;
	*(ptr + gfx_screen_tile_w) = id2 + 32;
	*(ptr + gfx_screen_tile_w + 1) = id2 + 33;

	//LOGD("GFX\t drawing object [%d] using tile [%d]\n",obj->tile,id);
}

void gfx_tileset_to_vram(struct gfx_tileset *ts, byte bank)
{
	uint offset, length;

	offset = 256 * 8 * bank + ts->start * 8;
	length = (ts->end - ts->start + 1) * 8;
	vdp_copy_to_vram(ts->bank->pattern, vdp_base_chars_grp1 + offset,
			 length);
	vdp_copy_to_vram(ts->bank->color, vdp_base_color_grp1 + offset, length);
}

void gfx_tilemap_clip(struct gfx_tilemap *tm,
		      struct gfx_viewport *vp, byte * scrbuf,
		      struct gfx_map_pos *p)
{
	byte i;
	byte *ptr = scrbuf + vp->x + vp->y * gfx_screen_tile_w;
	byte *src = tm->map + p->x + p->y * tm->w;

	for (i = 0; i <= vp->h; i++) {
		sys_memcpy(ptr, src, vp->w);
		ptr += gfx_screen_tile_w;
		src += tm->w;
	}
}
