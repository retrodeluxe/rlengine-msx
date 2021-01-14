/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
#include "gen/bitmap_test.h"
#include <stdlib.h>

void main()
{
  uint16_t dst;
  uint8_t *src,i;
  VdpRGBColor *pal;

	vdp_set_mode(MODE_GRP4);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);

  pal = (VdpRGBColor *)knight_pal_palette;

  for (i = 0; i <8; i++)
      vdp_set_palette_color(i, pal++);

  dst = VRAM_BASE_GRP4_PTRN;
  src = knight_bitmap;
  for (i = 0; i < knight_bitmap_h; i ++) {
    vdp_memcpy(dst, src, knight_bitmap_w / 2);
    dst += 128;
    src += knight_bitmap_w /2;
  }

  for(;;);
}
