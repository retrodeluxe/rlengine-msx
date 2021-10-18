/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */

#include "msx.h"
#include "log.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"
#include "blit.h"
#include "phys.h"

#include "celeste.h"

#include "gen/tileset_ext.h"
#include "gen/font_ext.h"
#include "gen/celeste_pal.h"
#include "gen/player.h"
#include "gen/snow_small.h"
#include "gen/snow_big.h"
#include "gen/dust.h"

/** Blit Sets */
BlitSet tiles_bs;
BlitSet font_bs;

const uint8_t player_state[] = {4, 1, 1, 1, 1, 1, 1, 4};
const uint8_t snow_state[] = {1};
const uint8_t dust_state[] = {3};

/** RAM palette */
VdpRGBColor palette[MAX_COLORS];

void init_pal()
{
  sys_memcpy((uint8_t *)palette, celeste_pal_palette, sizeof(VdpRGBColor) * MAX_COLORS);

  // workaround for zero color issue
  palette[1].r = 0;
  palette[1].g = 0;
  palette[1].b = 0;
}

void init_gfx()
{
  uint8_t i;

  vdp_set_mode(MODE_GRP4);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_screen_disable();
  spr_init2(SPR_SIZE_8, SPR_ZOOM_ON);
  vdp_clear(0);

  tile_init();
  phys_init();

  vdp_set_palette(palette);

  sys_set_rom();
  {
    INIT_BLIT_SET(tiles_bs, tileset, 16, 16);
    blit_set_to_vram(&tiles_bs, 3, 0, 0);
  }
  sys_set_bios();

  sys_set_rom();
  {
    sys_memcpy(map_data, MAP_DATA, 1024);
  }
  sys_set_bios();

  // font needs to be copied from ROM page3 to RAM before transfer
  font_bs.w = 256;
  font_bs.h = 96;
  font_bs.bitmap = rom_buffer;
  font_bs.raw = false;
  font_bs.allocated = false;
  font_bs.frame_w = 16;
  font_bs.frame_h = 16;
  sys_memcpy_rom(rom_buffer, font_bitmap, 4320);
  blit_set_to_vram(&font_bs, 2, 0, 0);

  //for (i = 1; i < 7; i++)
  //  phys_set_colliding_tile(i);
  for (i = 16; i < 16 + 8; i++)
     phys_set_colliding_tile(i);
  for (i = 32; i < 32 + 8; i++)
  //   phys_set_colliding_tile(i);
  // for (i=96; i<96+16; i++)
  //   phys_set_colliding_tile(i);

  /* init objects */
  // sys_set_rom();
  // {
  // INIT_DYNAMIC_TILE_SET(objects_ts[TILE_SPIKES], spikes, 2, 2, 4, 1);
  // INIT_DYNAMIC_TILE_SET(objects_ts[TILE_FAKE_WALL], fake_wall, 4, 4, 1, 1);
  // }
  // sys_set_bios();

  /* init sprites */
  SPR_DEFINE_PATTERN_SET(PATRN_PLAYER, SPR_SIZE_8x8, 2, 8, player_state, player);
  SPR_DEFINE_PATTERN_SET(PATRN_SNOW_SMALL, SPR_SIZE_8x8, 1, 1, snow_state, snow_small);
  SPR_DEFINE_PATTERN_SET(PATRN_SNOW_BIG, SPR_SIZE_8x8, 1, 1, snow_state, snow_big);
  SPR_DEFINE_PATTERN_SET(PATRN_DUST, SPR_SIZE_8x8, 1, 1, dust_state, dust);
}
