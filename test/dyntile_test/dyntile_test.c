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
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include <stdlib.h>
#include "dpo.h"
#include "phys.h"
#include "list.h"

#include "gen/dyntile_test.h"

struct tile_set giant_ts;
struct tile_set tileset_kv;
struct tile_object giant_tobj;

uint8_t scr_buf[768];

void main()
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	sys_irq_init();

	tile_init();

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	INIT_DYNAMIC_TILE_SET(giant_ts,giant, 4, 7, 2, 2);
	tile_set_valloc(&giant_ts);
	giant_tobj.x = 100;
	giant_tobj.y = 50;
	giant_tobj.cur_dir = 1;
	giant_tobj.cur_anim_step = 0;
	giant_tobj.ts = &giant_ts;
	giant_tobj.idx = 0;

	do {
		sys_memset(scr_buf, 0, 768);
		if (giant_tobj.cur_anim_step < giant_tobj.ts->n_frames - 1) {
			giant_tobj.cur_anim_step++;
		} else {
			giant_tobj.cur_anim_step = 0;
		}
		giant_tobj.x-=8;
		tile_object_show(&giant_tobj, scr_buf, true);

		vdp_copy_to_vram(scr_buf, vdp_base_names_grp1, 768);
		sys_sleep_ms(500);
	} while(1);
}
