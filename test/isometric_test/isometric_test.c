/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
#include "tile.h"
#include "tileblit.h"
#include "sprite.h"

#include "gen/dome.h"
#include "gen/wallcave.h"
#include "gen/floortile.h"

TileSet dome_ts;
TileSet floor_ts;
TileSet walls_ts;

uint8_t scr_pixel_buf[6144];

TileObject object[25];

void main()
{
  uint8_t i,j;
  uint16_t offset = 0;

  vdp_set_mode(MODE_GRP2);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(COLOR_BLUE);

  tile_init();

  //INIT_TILE_SET(dome_ts, dome);

  //
  INIT_TILE_SET(floor_ts, floortile);
  INIT_TILE_SET(walls_ts, wallcave);

  INIT_RAW_DYNAMIC_TILE_SET(dome_ts, dome, 4, 4, 1, 1);

  for (i=0; i<14; i++) {
    object[i].x = i * 16;
    object[i].y = i * 8;
    object[i].state = 0;
    object[i].frame = 0;
    object[i].tileset = &dome_ts;
    tileblit_object_show(&object[i], scr_pixel_buf);
  }

  vdp_memcpy(VRAM_BASE_PTRN, scr_pixel_buf, 6144);
  vdp_memset(VRAM_BASE_COLR, 6144, 0x0F);

  for (i = 0; i < 255; i++) {
     vdp_poke_names(offset++, i);
  }
  offset++;
  for (i = 0; i < 255; i++) {
     vdp_poke_names(offset++, i);
  }
  offset++;
  for (i = 0; i < 255; i++) {
     vdp_poke_names(offset++, i);
  }
  for(;;);
}
