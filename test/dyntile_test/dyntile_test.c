/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include <stdlib.h>
#include "dpo.h"
#include "phys.h"
#include "list.h"

#include "gen/dyntile_test.h"

TileObject giant_tobj;

uint8_t scr_buf[768];

void main()
{
	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);
	sys_irq_init();

	tile_init();
	tile_load_defs();

	tile_to_vram(TS_KVALLEY, 1);

	// show something regarding this one

	tile_valloc(TS_GIANT);
	giant_tobj.x = 100;
	giant_tobj.y = 50;
	giant_tobj.state = 0;
	giant_tobj.frame = 0;
	giant_tobj.tileset = &tile_sets[TS_GIANT];
	giant_tobj.idx = 0;

	// modify tob to accept also the index?

	do {
		sys_memset(scr_buf, 0, 768);
		if (giant_tobj.frame < giant_tobj.tileset->frames - 1) {
			giant_tobj.frame++;
		} else {
			giant_tobj.frame = 0;
		}
		giant_tobj.x-=8;
		tile_object_show(&giant_tobj, scr_buf, true);

		vdp_memcpy(VRAM_BASE_NAME, scr_buf, 768);
		sys_sleep_ms(500);
	} while(1);
}
