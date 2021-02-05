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
#include "sprite.h"

#include "gen/pointer.h"

SpriteDef pointer_spr;

#define PATRN_POINTER 0
const uint8_t pointer_states[] = {1};

void main()
{
  int16_t xpos, ypos;
  uint16_t mouse_offset;
  int8_t offset_x, offset_y;

	vdp_set_mode(MODE_GRP1);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

  spr_init();
  SPR_DEFINE_PATTERN_SET(PATRN_POINTER, SPR_SIZE_16x16, 1, 1, pointer_states, pointer);
  spr_valloc_pattern_set(PATRN_POINTER);

  xpos = 120; ypos = 100;
  spr_init_sprite(&pointer_spr, PATRN_POINTER);
  spr_set_pos(&pointer_spr, xpos, ypos);
  spr_show(&pointer_spr);

	sys_irq_init();

	for (;;) {
      sys_wait_vsync();
      spr_refresh();
      mouse_offset = sys_get_mouse(12);
      offset_x = (int8_t)(mouse_offset & 0x00FF);
      offset_y = (int8_t)((mouse_offset & 0xFF00) >> 8);

      xpos += offset_x; ypos += offset_y;
      if (xpos < 0) xpos = 0;
      if (xpos > 255) xpos = 255;
      if (ypos < 0) ypos = 0;
      if (ypos > 191) ypos = 191;

      spr_set_pos(&pointer_spr, xpos, ypos);
      spr_update(&pointer_spr);
	}
}
