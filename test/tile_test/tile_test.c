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

struct tile_set logo;
struct tile_set kv;

byte fb[768];

void main()
{
	byte i,x,y;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	/*
	 * load a tile set and generate a tile map manually
	 */

	INIT_TILE_SET(logo, retro_logo);
	tile_set_to_vram(&logo, 0);

	i = 0;
	for (y=0; y < retro_logo_tile_h; y++)
		for (x=0; x < retro_logo_tile_w; x++)
			vdp_poke(vdp_base_names_grp1 + 11 + 9 * 32 + x + y * 32, i++);

	do {
	} while (sys_get_key(8) & 1);
 
	/*
	 * load a pre-processed tile map
	 */

	INIT_TILE_SET(kv, kingsvalley);
	tile_set_to_vram(&kv, 0);

	i = 0;
	for (i=0; i < 255; i++)
		vdp_poke(vdp_base_names_grp1 + i, i);

	sys_memcpy(fb,tilemap,768);
	vdp_fastcopy_nametable(fb);

	do {
	} while (1);
}


