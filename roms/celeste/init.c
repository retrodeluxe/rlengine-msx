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
#include "ascii8.h"

#include "celeste.h"

#include "init.h"
#include "pico8sfx.h"

#include "gen/tileset_ext.h"
#include "gen/font_ext.h"
#include "gen/celeste_pal.h"
#include "gen/celeste_sprites_ext.h"

extern const uint8_t core_sfx[];
extern const uint8_t core_music[];

/** Blit Sets */
BlitSet tiles_bs;
BlitSet font_bs;

/** RAM palette */
VdpRGBColor palette[MAX_COLORS];

#pragma CODE_PAGE 3

void init_pal()
{
  sys_memcpy((uint8_t *)palette, celeste_pal_palette, sizeof(VdpRGBColor) * MAX_COLORS);

  // workaround for zero color issue
  palette[1].r = 0;
  palette[1].g = 0;
  palette[1].b = 0;

  log_e("01\n");
}

void init_sfx()
{
  ascii8_set_data(9);
//  unpack_sfx(core_sfx);
  // unpack_music(core_music);
}

void init_gfx()
{
  uint8_t i;

  vdp_set_mode(MODE_GRP4);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_screen_disable();
  spr_init2(SPR_SIZE_8, SPR_ZOOM_ON);
  vdp_clear(0);

  phys_init();
  vdp_set_palette(palette);

  ascii8_set_data(6);
  INIT_BLIT_SET(tiles_bs, tileset, 16, 16);
  blit_set_to_vram(&tiles_bs, 3, 0, 0);

  ascii8_set_data(8);
  INIT_BLIT_SET(font_bs, font, 16, 16);
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
  ascii8_set_data(9);
  SPR_DEFINE_PATTERN_SET(PATRN_PLAYER, SPR_SIZE_8x8, 2, 8, player_state, player);
  SPR_DEFINE_PATTERN_SET(PATRN_SNOW_SMALL, SPR_SIZE_8x8, 1, 1, snow_state, snow_small);
  SPR_DEFINE_PATTERN_SET(PATRN_SNOW_BIG, SPR_SIZE_8x8, 1, 1, snow_state, snow_big);
  SPR_DEFINE_PATTERN_SET(PATRN_DUST, SPR_SIZE_8x8, 1, 1, dust_state, dust);
  SPR_DEFINE_PATTERN_SET(PATRN_HAIR_BIG, SPR_SIZE_8x8, 1, 1, hair_state, hair_big);
  SPR_DEFINE_PATTERN_SET(PATRN_HAIR_SMALL, SPR_SIZE_8x8, 1, 1, hair_state, hair_small);
}
