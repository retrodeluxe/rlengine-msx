/**
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 */
#include "msx.h"
#include "vdp.h"
#include "tile.h"
#include "sys.h"
#include "gen/vdp_test.h"


TileSet tileset_kv;

uint8_t buffer[768];

void main()
{
  uint16_t i;

	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

  /* fill buffer with pattern */
  for (i = 0; i < 768; i+=2) {
		*(buffer + i) = 1;
		*(buffer + i + 1) = 2;
	}

  /* this should show a stable pattern on screen in both msx1 and msx2 */
	do {
		vdp_memcpy(VRAM_BASE_NAME, buffer, 768);
	} while (sys_get_key(8) & 1);

  do {
		// FIXME: Current implementation is too fast for tms9918
		vdp_fastcopy_nametable(buffer);
	} while (sys_get_key(8) & 1);
}
