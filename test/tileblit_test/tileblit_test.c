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

#include "gen/background.h"
#include "gen/room1.h"
#include "gen/room1_init.h"
#include "gen/box.h"

TileSet bg_ts;
TileSet box_ts;
TileSet walls_ts;

uint8_t scr_pixel_buf[6144];
uint8_t scr_buf[768];

TileObject tob_box;

void main()
{
   uint8_t j;
   uint16_t i, offset = 0;

   vdp_set_mode(MODE_GRP2);
   vdp_set_color(COLOR_WHITE, COLOR_BLACK);
   vdp_clear(COLOR_BLUE);

   tile_init();
   mem_init();

   INIT_RAW_TILE_SET(bg_ts, background);
   INIT_RAW_DYNAMIC_TILE_SET(box_ts, box, 4, 3, 1, 1);

   tob_box.tileset = &box_ts;
   tob_box.x = 100;
   tob_box.y = 100;
   tob_box.state = 0;
   tob_box.frame = 0;
   tob_box.idx = 0;

   tile_set_valloc(&bg_ts);

   for (i = 0, j = 0; i < 768; i++, j++) { 
      offset++;
      if (j > 31) {
         offset += 32;
         j = 0;
      }
      scr_buf[i] = room1_tilemap[offset];
   }

   for (i = 0; i < 256; i++) {
      scr_buf[i] = i; 
   }


   tileblit_object_show(&tob_box, &bg_ts, scr_buf, false);

   vdp_fastcopy_nametable(scr_buf);

   for(;;);
}
