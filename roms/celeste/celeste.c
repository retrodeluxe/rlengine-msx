/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */

#include "dpo.h"
#include "font.h"
#include "list.h"
#include "log.h"
#include "map.h"
#include "msx.h"
#include "phys.h"
#include "pt3.h"
#include "sfx.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"
#include "blit.h"

#include <stdlib.h>

#include "celeste.h"
#include "init.h"
#include "anim.h"
#include "scene.h"

/** rom buffer */
uint8_t rom_buffer[4320];

/** screen buffer 16x16 tiles */
uint8_t scr_buffer[512];

/** colission buffer 32x32 tiles */
uint8_t col_buffer[1024];

/** current map data buffer */
uint8_t map_data[1024];

/** main display list **/
List display_list;

/* dpo iterator */
List *elem, *elem2;
DisplayObject *dpo;

uint8_t rx, ry, refresh, y_offset;
uint8_t stick, trigger_a, trigger_b;
uint16_t reftick;
bool fps_stall;

void load_room(uint8_t x, uint8_t y);
void animate_all();
void show_intro();

void main()
{
  sys_disable_kbd_click();
  sys_rand_init((uint8_t *)&main);

  init_pal();
  init_gfx();
  init_animators();

  sys_irq_init();

  show_intro();

  rx = 0; ry = 0, refresh = 1; y_offset = 0;

  vdp_screen_disable();
  for(;;) {
    sys_irq_enable();
    sys_wait_vsync();
    spr_refresh();

    reftick = sys_get_ticks();

    stick = sys_get_stick(0) | sys_get_stick(1);
    trigger_a = sys_get_trigger(0) | sys_get_trigger(1);
    trigger_b = sys_get_trigger(3);// | ~(sys_get_keyrow(4) & 0x08); /* N */


    if (refresh) {
     load_room(rx, ry);
     refresh = 0;
    }

    // if (stick == STICK_DOWN) {
    //   if (y_offset < 42) y_offset++;
    //   vdp_set_vert_offset(y_offset);
    // } else if (stick == STICK_UP) {
    //   if (y_offset > 0) y_offset--;
    //   vdp_set_vert_offset(y_offset);
    // }

    animate_all();

    fps_stall = true;
    while (sys_get_ticks() - reftick < 1) {
      fps_stall = false;
    }
  }
}

void animate_all() {
  list_for_each(elem, &display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    list_for_each(elem2, &dpo->animator_list) {
      anim = list_entry(elem2, Animator, list);
      anim->run(dpo);
    }
  }
}
