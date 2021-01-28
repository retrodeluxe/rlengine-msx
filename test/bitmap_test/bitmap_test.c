/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
#include "blit.h"
#include "ascii8.h"
#include <stdlib.h>

#include "gen/abbaye_map.h"
#include "gen/knight.h"
#include "gen/knight_pal.h"
#include "gen/abbaye_pal.h"
#include "gen/abbaye_ext.h"

#pragma CODE_PAGE 3

uint16_t dst, reftick;
bool fps_stall;

BlitSet knight_bs;
BlitSet abbaye_bs;
BlitObject knight_bo[9];

VdpCommand cmd;
uint8_t scr_buf[768];

void main() __nonbanked
{
  uint8_t i,j,x, offset_x, offset_y, anim_ctr;
  uint8_t *dst, *src;
  VdpRGBColor *pal;

  vdp_set_mode(MODE_GRP4);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);

  vdp_set_palette((VdpRGBColor *)abbaye_pal_palette);

  INIT_DYNAMIC_BLIT_SET(knight_bs, knight, 16, 24, 2, 2);
  blit_set_to_vram(&knight_bs, 3, 0, 0);

  ascii8_get_page(abbaye_bitmap);
  ascii8_set_data(ascii8_page);
  INIT_BLIT_SET(abbaye_bs, abbaye, 8, 8);
  blit_set_to_vram(&abbaye_bs, 2, 0, 0);

  sys_irq_init();
  sys_irq_enable();

  vdp_screen_disable();
  reftick = sys_get_ticks();

  blit_map_tilebuffer(abbaye_map_map + 64 * 22 + 32, &abbaye_bs, 1);
  blit_map_tilebuffer(abbaye_map_map + 64 * 22 + 32, &abbaye_bs, 0);
  vdp_screen_enable();
  vdp_set_192_lines();
  vdp_sprite_disable();

  log_e("took %d ticks \n", sys_get_ticks() - reftick);

  for (i = 0; i < 5; i++) {
    knight_bo[i].x = 10 + i * 24;
    knight_bo[i].y = 136;
    knight_bo[i].prev_x = 10 + i * 24;
    knight_bo[i].prev_y = 136;
    knight_bo[i].state = 1;
    knight_bo[i].frame = 0;
    knight_bo[i].blitset = &knight_bs;
    knight_bo[i].mask_x = 100 + i * 32;
    knight_bo[i].mask_y = 0;
    knight_bo[i].mask_page = 3;
    knight_bo[i].anim_ctr = i % 2;
    blit_object_show(&knight_bo[i]);
  }

  for (x = 10; x < 150; x++) {
    for (i =0 ; i < 5; i++) {
      knight_bo[i].x++;
      if (knight_bo[i].anim_ctr++ > 10) {
        if(++knight_bo[i].frame >= knight_bo[i].blitset->frames)
            knight_bo[i].frame = 0;
        knight_bo[i].anim_ctr = 0;
      }
      blit_object_update(&knight_bo[i]);
    }
  }

  for(;;);
}
