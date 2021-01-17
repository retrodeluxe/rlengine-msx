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
#include "gen/bitmap_test.h"
#include <stdlib.h>


uint16_t dst;

void main()
{
  uint8_t i;
  VdpRGBColor *pal;

  BlitSet knight_bs;
  BlitObject knight_bo;

  vdp_set_mode(MODE_GRP4);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);

  pal = (VdpRGBColor *)knight_pal_palette;

  for (i = 0; i <8; i++)
      vdp_set_palette_color(i, pal++);

  INIT_DYNAMIC_BLIT_SET(knight_bs, knight, 16, 24, 2, 2);

  blit_set_to_vram(&knight_bs, 1, 0, 0);

  knight_bo.x = 100;
  knight_bo.y = 100;
  knight_bo.state = 0;
  knight_bo.frame = 0;
  knight_bo.blitset = &knight_bs;

  blit_object_show(&knight_bo);

  for(;;);
}
