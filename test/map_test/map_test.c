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
#include "log.h"
#include "map.h"
#include "gen/map_test.h"
#include <stdlib.h>

struct tile_set logo;
struct tile_set kv;

uint8_t fb[768];
uint8_t map_buf[8000];

void main()
{
	uint8_t i,x,y, d;
	uint8_t *ptr;
	uint8_t *src;
	unsigned int ct;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	sys_irq_init();


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

	INIT_TILE_SET(kv, tiles2);
	// need an offset of one
	tile_set_to_vram(&kv, 1);

	// inflate map into a buffer, at once
 	map_inflate(map_cmpr_dict, map, map_buf, 2000, map_w);

	x =0; y =0;
    do {
    	d = sys_get_stick(0);
    	if (d == 1 && y > 0)
    		y--;
    	if (d == 5 && y < (map_h - 26))
    		y++;
    	if (d == 3 && x < map_w - 32)
    		x++;
    	if (d == 7 && x > 0)
    		x--;

		ptr = fb;
		src = map_buf + x + y * map_w;
		for (i = 0; i <= 23; i++) {
			sys_memcpy(ptr, src, 32);
			ptr += gfx_screen_tile_w;
			src += map_w;
		}

		//map_inflate_screen(map_cmpr_dict, map, fb, map_w, x, y);
		vdp_fastcopy_nametable(fb);

	} while(1);
}


