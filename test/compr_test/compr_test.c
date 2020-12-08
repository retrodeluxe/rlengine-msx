/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"
#include <stdlib.h>

#include "gen/compr_test.h"

TileSet tileset_palette;
TileSet tileset_logo;
TileSet tileset_kv;
TileSet tileset_intro;

uint8_t map_buf[768];

/**
 * compr_test: test tile RLE decompression with different tile sets
 *
 */
void main()
{
	uint8_t x,y;
	uint16_t i;

	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

	INIT_TILE_SET(tileset_palette, palette);
	tile_set_to_vram(&tileset_palette, 0);

	for (i=0; i< 255; i++)
		vdp_poke_names(i, i);

	do {
	} while (sys_get_key(8) & 1);

	INIT_TILE_SET(tileset_intro, intro_tileset);
	tile_set_to_vram(&tileset_intro, 0);

	for (i=0; i< 255; i++)
		vdp_poke_names(i, i);

	do {
	} while (sys_get_key(8) & 1);


	/*
	 * load a tile set and generate a tile map manually
	 */
	INIT_TILE_SET(tileset_logo, retro_logo);
	tile_set_to_vram(&tileset_logo, 0);

	i = 0;
	for (y = 0; y < retro_logo_tile_h; y++)
		for (x = 0; x < retro_logo_tile_w; x++)
			vdp_poke_names(11 + 9 * 32 + x + y * 32, i++);

	do {
	} while (sys_get_key(8) & 1);

	/*
	 * load a pre-processed tile map (uncompressed)
	 */
	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1 /* offset of 1 */);

	for (i = 0; i < 768; i++)
		vdp_poke_names(i, *(map_tilemap + i));

	do {
	} while (1);
}
