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
#include "ascii8.h"

#include <stdlib.h>

#include "gen/celeste_tiles_ext.h"
#include "gen/celeste_raw_tiles_ext.h"
#include "gen/player.h"

#pragma CODE_PAGE 3

extern const uint8_t MAP_DATA[8192];

#define LEVEL_IDX = (room.x % 8 + room.y * 8)
#define SCENE_MAX_DPO 15

enum map_tile_t {
    TYPE_PLAYER_SPAWN = 2,
    TYPE_PLAYER,
    TYPE_PLATFORM,
    TYPE_KEY = 9,
    TYPE_PLATFORM_A = 12,
    TYPE_PLATFORM_B,
    TYPE_VERTICAL_SPIKES = 18,
    TYPE_SPRING = 19,
    TYPE_CHEST = 21,
    TYPE_BALLOON = 23,
    TYPE_FALL_FLOOR = 24,
    TYPE_FRUIT = 25,
    TYPE_FLY_FRUIT = 29,
    TYPE_FAKE_WALL = 65,
    TYPE_MESSAGE = 87,
    TYPE_BIG_CHEST = 97,
    TYPE_FLAG = 119,
    TYPE_SMOKE,
    TYPE_ORB,
    TYPE_LIFEUP
};

enum map_object_t {
    TILE_SPIKES,
    TILE_FAKE_WALL,
    TILE_CHEST,
    TILE_KEY,
    TILE_SPRING,
    TILE_FALL_FLOOR,
    TILE_OBJECT_MAX
};

enum spr_patterns_t {
    PATRN_PLAYER,
    PATRN_SMOKE,
    PATRN_BALLOON,
    PATRN_CLOUD,
    PATRN_MAX
};

TileSet terrain_ts;
TileSet objects_ts[TILE_OBJECT_MAX];
uint8_t scr_buffer[1024];

/** scene primitives **/
TileObject tileobject[SCENE_MAX_DPO];

/** temporary dpo **/
DisplayObject *tmp_dpo;

/** main display list **/
List display_list;

/* Elmenet iterator */
List *elem, *elem2;

/** scene display objects **/
DisplayObject display_object[SCENE_MAX_DPO];
DisplayObject *dpo;

/** main character display object **/
DisplayObject dpo_player;

SpriteDef player_spr;

uint8_t dpo_ct;

enum anim_t {
    ANIM_PLAYER,
    MAX_ANIMATORS,
};

enum player_state {
    STATE_IDLE,
    STATE_MOVING_LEFT,
    STATE_MOVING_RIGHT,
    STATE_JUMPING,
    STATE_CROUCHING,
    STATE_LOOKING_UP,
    STATE_FALLING,
    STATE_COLLISION,
    STATE_DEATH,
};

/* this needs to match sprite definition */
enum player_anim_state {
    PLAYER_ANIM_RIGHT,
    PLAYER_ANIM_HANG_RIGHT,
    PLAYER_ANIM_CROUCH_RIGHT,
    PLAYER_ANIM_UP_RIGHT,
    PLAYER_ANIM_UP_LEFT,
    PLAYER_ANIM_CROUCH_LEFT,
    PLAYER_ANIM_HANG_LEFT,
    PLAYER_ANIM_LEFT,
};

Animator animators[MAX_ANIMATORS];
Animator *anim;

uint8_t rx, ry, refresh;
uint8_t stick, trigger_a, trigger_b;
uint8_t *tmp;
uint16_t i;
uint16_t reftick;
bool fps_stall;

void load_room(uint8_t x, uint8_t y);
void init_gfx();

void animate_all() __nonbanked;
void init_animators();

void main() __nonbanked {

  vdp_set_mode(MODE_GRP2);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(0);

  sys_disable_kbd_click();
  sys_rand_init((uint8_t *)&main);

  // show retrodeluxe logo

  init_gfx();
  init_animators();
  sys_irq_init();

  rx = 0; ry = 0, refresh = 1;

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


    animate_all();

    fps_stall = true;
    while (sys_get_ticks() - reftick < 1) {
      fps_stall = false;
    }
       vdp_memcpy(VRAM_BASE_NAME, scr_buffer + 256, 768);
  }

}

void animate_all() __nonbanked {
  list_for_each(elem, &display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    list_for_each(elem2, &dpo->animator_list) {
      anim = list_entry(elem2, Animator, list);
      anim->run(dpo);
    }
  }
}

#define DX 2
#define DY_JUMP 8
#define DX_JUMP 4
#define DY_FALL 1

void anim_player(DisplayObject *obj) __nonbanked {
    int8_t dx, dy;
    static int16_t jmp =0;
    uint8_t x, y;

    SpriteDef *sp = obj->spr;
    SpritePattern *ps = sp->pattern_set;

    dx = 0;
    dy = 0;

    x = (sp->planes[0]).x;
    y = (sp->planes[0]).y;

    switch(obj->state) {
        case STATE_IDLE:
          jmp = 0;
          if (stick == STICK_LEFT) {
            obj->state = STATE_MOVING_LEFT;
          } else if (stick == STICK_RIGHT) {
            obj->state = STATE_MOVING_RIGHT;
          } else if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT ||
                     stick == STICK_DOWN_RIGHT) {
            obj->state = STATE_CROUCHING;
            if (sp->state == PLAYER_ANIM_LEFT)
              sp->state = PLAYER_ANIM_CROUCH_LEFT;
            else if (sp->state == PLAYER_ANIM_RIGHT)
              sp->state = PLAYER_ANIM_CROUCH_RIGHT;
          } else if (stick == STICK_UP) {
            obj->state = STATE_LOOKING_UP;
            if (sp->state == PLAYER_ANIM_LEFT)
              sp->state = PLAYER_ANIM_UP_LEFT;
            else if (sp->state == PLAYER_ANIM_RIGHT)
              sp->state = PLAYER_ANIM_UP_RIGHT;
          }
          if (trigger_a) {
            obj->state = STATE_JUMPING;
            jmp = -DY_JUMP;
          }
          if (trigger_b) {
            log_e("long jump\n");
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
        case STATE_JUMPING:
          dy = jmp++;
          if (jmp == 0)
            obj->state = STATE_FALLING;
          if (stick == STICK_LEFT) {
            dx = -DX_JUMP;
          } else if (stick == STICK_RIGHT) {
            dx = DX_JUMP;
          }
          if (is_colliding_up(obj)) {
            obj->state = STATE_FALLING;
            jmp = 0;
          }
          break;
        case STATE_FALLING:
          dy = jmp;
          if (jmp++ > 6)
            jmp = DY_FALL;
          if (stick == STICK_LEFT) {
            dx = -DX_JUMP;
          } else if (stick == STICK_RIGHT) {
            dx = DX_JUMP;
          }
          // we do not have jumping states, need to do it by hand
          // if (sp->state == PLAYER_ANIM_RIGHT ||
          //   sp->state == PLAYER_ANIM_CROUCH_RIGHT) {
          //   sp->state = JANE_STATE_RIGHT_JUMP;
          // } else if (sp->state == JANE_STATE_LEFT ||
          //            sp->state == JANE_STATE_LEFT_CROUCH) {
          //   sp->state = JANE_STATE_LEFT_JUMP;
          // }
          // if (stick == STICK_LEFT) {
          //   sp->state = JANE_STATE_LEFT_JUMP;
          //   dx = -JEAN_DX;
          // } else if (stick == STICK_RIGHT) {
          //   sp->state = JANE_STATE_RIGHT_JUMP;
          //   dx = JEAN_DX;
          // }
          if (is_colliding_down(obj)) {
            //if (sp->state == JANE_STATE_RIGHT_JUMP) {
            //  sp->state = JANE_STATE_RIGHT;
            //} else if (sp->state == JANE_STATE_LEFT_JUMP) {
            //  sp->state = JANE_STATE_LEFT;
            //}
            obj->state = STATE_IDLE;
          }
          break;
        case STATE_MOVING_LEFT:
          sp->state = PLAYER_ANIM_LEFT;
          if (stick == STICK_LEFT) {
            dx = -2;
          } else if (stick == STICK_RIGHT) {
            obj->state = STATE_MOVING_RIGHT;
          } else if (stick == STICK_CENTER) {
            obj->state = STATE_IDLE;
          } else if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT) {
            obj->state = STATE_CROUCHING;
            sp->state = PLAYER_ANIM_CROUCH_LEFT;
          } else if (stick == STICK_UP || stick == STICK_UP_LEFT) {
            obj->state = STATE_LOOKING_UP;
            sp->state = PLAYER_ANIM_UP_LEFT;
          }
          if (trigger_a) {
            obj->state = STATE_JUMPING;
            jmp = -DY_JUMP;
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
        case STATE_MOVING_RIGHT:
          sp->state = PLAYER_ANIM_RIGHT;
          if (stick == STICK_RIGHT) {
            dx = 2;
          } else if (stick == STICK_LEFT) {
            obj->state = STATE_MOVING_LEFT;
          } else if (stick == STICK_CENTER) {
            obj->state = STATE_IDLE;
          } else if (stick == STICK_DOWN || stick == STICK_DOWN_RIGHT) {
            obj->state = STATE_CROUCHING;
            sp->state = PLAYER_ANIM_CROUCH_RIGHT;
          } else if (stick == STICK_UP || stick == STICK_UP_RIGHT) {
            obj->state = STATE_LOOKING_UP;
            sp->state = PLAYER_ANIM_UP_RIGHT;
          }
          if (trigger_a) {
            obj->state = STATE_JUMPING;
            jmp = -DY_JUMP;
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
        case STATE_CROUCHING:
          if (stick == STICK_CENTER) {
            if (sp->state == PLAYER_ANIM_CROUCH_LEFT)
              sp->state = PLAYER_ANIM_LEFT;
            else if (sp->state == PLAYER_ANIM_CROUCH_RIGHT)
              sp->state = PLAYER_ANIM_RIGHT;
            obj->state = STATE_IDLE;
          } else if (stick == STICK_DOWN_LEFT) {
            dx = -2;
            sp->state = PLAYER_ANIM_CROUCH_LEFT;
          } else if (stick == STICK_DOWN_RIGHT) {
            dx = 2;
            sp->state = PLAYER_ANIM_CROUCH_RIGHT;
          } else if (stick == STICK_LEFT) {
            obj->state = STATE_MOVING_LEFT;
          } else if (stick == STICK_RIGHT) {
            obj->state = STATE_MOVING_RIGHT;
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
        case STATE_LOOKING_UP:
          if (stick == STICK_CENTER) {
            obj->state = STATE_IDLE;
            if (sp->state == PLAYER_ANIM_UP_LEFT)
              sp->state = PLAYER_ANIM_LEFT;
            else if (sp->state == PLAYER_ANIM_UP_RIGHT)
              sp->state = PLAYER_ANIM_RIGHT;
          } else if (stick == STICK_UP_LEFT) {
            dx = -2;
            sp->state = PLAYER_ANIM_UP_LEFT;
          } else if (stick == STICK_UP_RIGHT) {
            dx = 2;
            sp->state = PLAYER_ANIM_UP_RIGHT;
          } else if (stick == STICK_LEFT) {
            obj->state = STATE_MOVING_LEFT;
          } else if (stick == STICK_RIGHT) {
            obj->state = STATE_MOVING_RIGHT;
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
    }

    phys_detect_tile_collisions(obj, scr_buffer + 256, dx, dy, false, true);

    if (obj->state != STATE_IDLE) {
      sp->anim_ctr++;
      if (sp->anim_ctr > sp->anim_ctr_treshold) {
        sp->frame++;
        sp->anim_ctr = 0;
      }
      if (sp->frame > ps->state_steps[sp->state] - 1)
        sp->frame = 0;
    }

    if ((dx > 0 && !is_colliding_right(obj)) ||
       (dx < 0 && !is_colliding_left(obj))) {
        obj->xpos += dx;
    }
    if ((dy > 0 && !is_colliding_down(obj)) ||
       (dy < 0 && !is_colliding_up(obj))) {
        obj->ypos += dy;
    }

    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
    // do nothing for now
    // log_e("anim player\n");

    // here I can update the playes sprite and start experiementing
    // with the animations,

    // this is actually most of the meat.
    //

}

void init_animators() {
  uint8_t i;

  animators[ANIM_PLAYER].run = anim_player;
}

const uint8_t player_state[] = {4, 1, 1, 1, 1, 1, 1, 4};

void init_gfx() {
  uint8_t i;
  tile_init();
  phys_init();

  spr_init2(SPR_SIZE_8, SPR_ZOOM_ON);

  // FIXME: valloc not working properly
  ascii8_set_data(7);
  INIT_RAW_TILE_SET(terrain_ts, terrain);
  tile_set_to_vram(&terrain_ts, 0);

  for (i=1; i<16; i++)
    phys_set_colliding_tile(i);
  for (i=32; i<32+16; i++)
    phys_set_colliding_tile(i);
  for (i=64; i<64+16; i++)
    phys_set_colliding_tile(i);
  for (i=96; i<96+16; i++)
    phys_set_colliding_tile(i);

  /* init objects */
  INIT_DYNAMIC_TILE_SET(objects_ts[TILE_SPIKES], spikes, 2, 2, 4, 1);
  INIT_DYNAMIC_TILE_SET(objects_ts[TILE_FAKE_WALL], fake_wall, 4, 4, 1, 1);

  /* init sprites */
  SPR_DEFINE_PATTERN_SET(PATRN_PLAYER, SPR_SIZE_8x8, 3, 8, player_state, player);
}


void add_tileobject(DisplayObject *dpo, uint8_t objidx, uint8_t x, uint8_t y,
                           enum tile_sets_t tileidx) __nonbanked {

  ascii8_set_data(7);
  if (RLE_COULD_NOT_ALLOCATE_VRAM == tile_set_valloc(&objects_ts[tileidx])) {
      log_e("this is bad\n");
  }

  tileobject[objidx].x = x * 16;
  tileobject[objidx].y = y * 16;
  tileobject[objidx].state = 0;
  tileobject[objidx].frame = 0;
  tileobject[objidx].tileset = &objects_ts[tileidx];
  tileobject[objidx].idx = 0;

  dpo->type = DISP_OBJECT_TILE;
  dpo->tob = &tileobject[objidx];
  dpo->xpos = x * 16;
  dpo->ypos = y * 16;
  dpo->visible = true;
  dpo->state = 0;

  INIT_LIST_HEAD(&dpo->list);
  list_add(&dpo->list, &display_list);
  INIT_LIST_HEAD(&dpo->animator_list);
  ascii8_set_data(6);
}

void add_animator(DisplayObject *dpo, enum anim_t animidx) {
  list_add(&animators[animidx].list, &dpo->animator_list);
}

void add_player(uint8_t x, uint8_t y) __nonbanked {
    if(!spr_valloc_pattern_set(PATRN_PLAYER)) {
        log_e("could not alloc sprites\n");
    }

    spr_init_sprite(&player_spr, PATRN_PLAYER);


    //log_e("set pos %d %d\n", x*16, y*16);
    dpo_player.xpos = x * 16;
    dpo_player.ypos = (y - 4) * 16;
    dpo_player.type = DISP_OBJECT_SPRITE;
    dpo_player.state = 0;
    dpo_player.spr = &player_spr;
    dpo_player.visible = true;
    dpo_player.collision_state = 0;
    dpo_player.check_collision = false;
    INIT_LIST_HEAD(&dpo_player.list);
    list_add(&dpo_player.list, &display_list);
    spr_set_pos(&player_spr, x * 16, (y - 4) * 16);
    INIT_LIST_HEAD(&dpo_player.animator_list);
    add_animator(&dpo_player, ANIM_PLAYER);

    // bit of a challenge is that certain player movements
    // will trigger scrolling... which in turn will affect player sprite
    // position. Not sure how this should be handled?

    // e.g fall and at some y start scrolling instead of moving the sprite
    // that is going to be super tricky to get right.
    // it shouldn't be a big problem though.?

}

void load_room(uint8_t x, uint8_t y) {

  uint8_t tx, ty, tile, tile2x = 0;
  uint8_t *dst = scr_buffer;
  uint16_t i = (x % 8 + y * 8) * 256; // 8192 - 256

  //log_e("index %d\n", i);
  ascii8_set_data(6);

  INIT_LIST_HEAD(&display_list);

  dpo_ct = 0;
  dpo = display_object;

  for (ty = 0; ty < 16; ty ++) {
    for (tx = 0; tx < 16; tx++) {
      tile = MAP_DATA[i] + 1;

      switch (tile) {
        case TYPE_VERTICAL_SPIKES:
          add_tileobject(dpo, dpo_ct, tx, ty, TILE_SPIKES);
          dpo++;
          dpo_ct++;
          break;
        case TYPE_FAKE_WALL:
          add_tileobject(dpo, dpo_ct, tx, ty, TILE_FAKE_WALL);
          dpo++;
          dpo_ct++;
          break;
        case TYPE_MESSAGE:
        case TYPE_BIG_CHEST:
        case TYPE_BIG_CHEST+1:
        case TYPE_KEY:
        case TYPE_CHEST:
        case TYPE_FRUIT:
        case TYPE_FLY_FRUIT:
          break;
        case TYPE_PLAYER_SPAWN:
          add_player(tx, ty);
         // break;
        case TYPE_FALL_FLOOR:
        case TYPE_SPRING:
        case TYPE_BALLOON:
        case TYPE_FLAG:
        case TYPE_PLATFORM_A:
        case TYPE_PLATFORM_B:
          tile = 1;
        default:
        // clean this up, use a table something cleaner please.
          if (tile == 1)
            tile2x = 51;
          else if (tile == 17)
            tile2x = 30;
          else if (tile > 32 && tile < 48) {
            tile2x = (tile - 33) * 2;
          } else if (tile > 47 && tile < 65) {
            tile2x = (tile - 17) * 2;
          } else if (tile > 64 && tile < 70) {
            tile2x = (tile - 65) * 2 + 128;
          } else if (tile > 80 && tile < 87) {
            tile2x = (tile + 97) * 2;
          } else if (tile > 98 && tile < 102) {
              // ice blocks here
          } else if (tile > 114 && tile < 119) {
              // ice blocks here
          } else if (tile == 73) {
            tile2x = 86;
          } else if (tile == 89) {
            tile2x = 22;
          } else if (tile == 104) {
            tile2x = 26;
          } else if (tile == 105) {
            tile2x = 28;
          }

          *dst = tile2x;
          *(dst+1) = tile2x + 1;
          *(dst+32) = tile2x + 32;
          *(dst+33) = tile2x + 33;
      }
      dst+=2;
      i++;
    }
    dst+=32;
    // clean line if needed?
  }

  list_for_each(elem, &display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
      spr_show(dpo->spr);
      //log_e("show spritesn\n");
    } else if (dpo->type == DISP_OBJECT_TILE && dpo->visible) {
    //  log_e("showing tob\n");
      tile_object_show(dpo->tob, scr_buffer, false);
    }
  }

  vdp_memcpy(VRAM_BASE_NAME, scr_buffer + 256, 768);
}
