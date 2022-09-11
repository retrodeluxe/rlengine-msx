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
#include "gen/room1_defs.h"
#include "gen/box.h"

TileSet bg_ts;
TileSet box_ts;
TileSet walls_ts;

uint8_t scr_pixel_buf[6144];
uint8_t scr_buf[768];

TileObject *dpo;

struct room1_object_item *map_object;

void main()
{
   uint8_t j, scene_idx;
   uint16_t i, offset = 0;

   vdp_set_mode(MODE_GRP2);
   vdp_set_color(COLOR_WHITE, COLOR_BLACK);
   vdp_clear(COLOR_BLUE);

   tile_init();
   mem_init();
   init_room1_object_layers();

   INIT_RAW_TILE_SET(bg_ts, background);
   INIT_RAW_DYNAMIC_TILE_SET(box_ts, box, 4, 3, 1, 1);
   tile_set_valloc(&bg_ts);

   for (i = 0, j = 0; i < 768; i++, j++) {
      offset++;
      if (j > 31) {
         offset += 32;
         j = 0;
      }
      scr_buf[i] = room1_tilemap[offset];
   }

   map_object = (struct room1_object_item *) room1_object_layer[0];
   for (scene_idx = 0; map_object->type != 255; scene_idx++) {
       switch(map_object->type) {
           case BOX:
            dpo = dpo_new();
            dpo->tileset = &box_ts;
            dpo->x = map_object->x /8 * 8;
            dpo->y = (map_object->y + 4) /8 * 8;
            dpo->state = 0;
            dpo->frame = 0;
            dpo->idx = 0;
            tileblit_object_show(dpo, &bg_ts, scr_buf, false);
            break;
           default:
       }
       map_object++;
   }

   vdp_fastcopy_nametable(scr_buf);

   for(;;);
}
