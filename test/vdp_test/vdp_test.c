/**
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 */
#include "msx.h"
#include "vdp.h"
#include "tile.h"
#include "sys.h"
#include "gen/vdp_test.h"


struct tile_set tileset_kv;

uint8_t buffer[768];

void main()
{
  uint16_t i;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

  /* fill buffer with pattern */
  for (i = 0; i < 768; i+=2) {
		*(buffer + i) = 1;
		*(buffer + i + 1) = 2;
	}

  /* this should show a stable pattern on screen in both msx1 and msx2 */
	do {
		  vdp_copy_to_vram(buffer, vdp_base_names_grp1, 768);
	} while (sys_get_key(8) & 1);

  do {
		// FIXME: Current implementation is too fast for tms9918
    vdp_fastcopy_nametable(buffer);
	} while (sys_get_key(8) & 1);
}
