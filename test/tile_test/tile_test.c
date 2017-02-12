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
#include "wq.h"
#include "tile.h"
#include "gen/tile_test.h"
#include <stdlib.h>
#include "map.h"

struct tile_set tileset_logo;
struct tile_set tileset_kv;

uint8_t map_buf[768];

void main()
{
	uint8_t x,y;
  uint16_t i;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	/*
	 * load a tile set and generate a tile map manually
	 */
	INIT_TILE_SET(tileset_logo, retro_logo);
	tile_set_to_vram(&tileset_logo, 0);

	i = 0;
	for (y = 0; y < retro_logo_tile_h; y++)
		for (x = 0; x < retro_logo_tile_w; x++)
			vdp_poke(vdp_base_names_grp1 + 11 + 9 * 32 + x + y * 32, i++);

	do {
	} while (sys_get_key(8) & 1);

	/*
	 * load a pre-processed tile map
	 */
	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1 /* offset of 1 */);

  map_inflate(tilemap_cmpr_dict, tilemap, map_buf, 2000, tilemap_w);

	for (i = 0; i < 768; i++)
		vdp_poke(vdp_base_names_grp1 + i, *(map_buf + i));

	do {
	} while (1);
}
