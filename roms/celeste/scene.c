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

#include "anim.h"
#include "celeste.h"
#include "scene.h"
#include "init.h"

/** scene display objects **/
DisplayObject display_object[SCENE_MAX_DPO];

/** main character display object **/
DisplayObject dpo_player;
DisplayObject dpo_dust[5];

SpriteDef player_spr;
SpriteDef dust_spr[5];
SpriteDef snow_spr[20];

uint8_t dust_ct;

uint8_t snow_status[20];
uint8_t snow_aux[20];
uint8_t snow_aux2[20];
int16_t snow_y[20];

uint8_t dpo_ct;

void add_snow(DisplayObject *obj, uint8_t sprid, enum spr_patterns_t pattidx)
{
  int16_t x,y;

  spr_valloc_pattern_set(pattidx);
  sys_irq_enable();
  spr_init_sprite(&snow_spr[sprid], pattidx);

  x = sprid * 16;
  y = sprid * 16 + sys_rand() & 127;

  spr_set_pos(&snow_spr[sprid], x, y);
  spr_show(&snow_spr[sprid]);

  obj->type = DISP_OBJECT_SPRITE;
  obj->spr = &snow_spr[sprid];
  obj->xpos = x;
  obj->ypos = y;
  obj->state = 0;
  obj->aux = (sys_rand() & 15) + 1;
  obj->aux2 = (sys_rand() & 3) + 1;

  obj->visible = true;
  obj->collision_state = 0;
  obj->check_collision = false;

  INIT_LIST_HEAD(&obj->list);
  list_add(&obj->list, &display_list);
  INIT_LIST_HEAD(&obj->animator_list);
}

void add_fast_snow_storm(DisplayObject *obj)
{
  int16_t x,y;
  uint8_t i, sprid = 0;

  spr_valloc_pattern_set(PATRN_SNOW_BIG);
  spr_valloc_pattern_set(PATRN_SNOW_SMALL);
  sys_irq_enable();

  obj->state = 0;

  for (i = 0; i < NUM_SNOW_SMALL; i++) {
    spr_init_sprite(&snow_spr[sprid], PATRN_SNOW_SMALL);
    x = sprid * 16;
    y = sprid * 16 + sys_rand() & 127;
    spr_set_pos(&snow_spr[sprid], x, y);
    spr_show(&snow_spr[sprid]);
    snow_status[sprid] = 0;
    snow_aux[sprid] = (sys_rand() & 15) + 1;
    snow_aux2[sprid] = (sys_rand() & 3) + 1;
    snow_y[sprid] = y;
    sprid++;
  }

  for (i = 0; i < NUM_SNOW_BIG; i++) {
    spr_init_sprite(&snow_spr[sprid], PATRN_SNOW_BIG);
    x = sprid * 16;
    y = sprid * 16 + sys_rand() & 127;
    spr_set_pos(&snow_spr[sprid], x, y);
    spr_show(&snow_spr[sprid]);
    snow_status[sprid] = 0;
    snow_aux[sprid] = (sys_rand() & 15) + 1;
    snow_aux2[sprid] = (sys_rand() & 3) + 1;
    snow_y[sprid] = y;
    sprid++;
  }

  INIT_LIST_HEAD(&obj->list);
  list_add(&obj->list, &display_list);
  INIT_LIST_HEAD(&obj->animator_list);
  add_animator(obj, ANIM_FAST_SNOW);
}


void add_player(uint8_t x, uint8_t y) {

  spr_valloc_pattern_set(PATRN_PLAYER);
  spr_init_sprite(&player_spr, PATRN_PLAYER);

  dpo_player.xpos = x * 16;
  dpo_player.ypos = 255;
  dpo_player.aux = y * 16;
  dpo_player.type = DISP_OBJECT_SPRITE;
  dpo_player.state = 0;
  dpo_player.spr = &player_spr;
  dpo_player.visible = true;
  dpo_player.collision_state = 0;
  dpo_player.check_collision = false;
  INIT_LIST_HEAD(&dpo_player.list);
  list_add(&dpo_player.list, &display_list);
  spr_set_pos(&player_spr, x * 16, y * 16);
  INIT_LIST_HEAD(&dpo_player.animator_list);
  add_animator(&dpo_player, ANIM_PLAYER_SPAWN);
}

void add_dust(uint16_t x, uint16_t y) {

  spr_valloc_pattern_set(PATRN_DUST);

  // FIXME: big problem with this.
  sys_irq_enable();

  spr_init_sprite(&dust_spr[dust_ct], PATRN_DUST);
  dpo_dust[dust_ct].xpos = x;
  dpo_dust[dust_ct].ypos = y + 4;
  dpo_dust[dust_ct].type = DISP_OBJECT_SPRITE;
  dpo_dust[dust_ct].state = 0;
  dpo_dust[dust_ct].spr = &dust_spr[dust_ct];
  dpo_dust[dust_ct].visible = true;
  dpo_dust[dust_ct].collision_state = 0;
  dpo_dust[dust_ct].check_collision = false;
  INIT_LIST_HEAD(&dpo_dust[dust_ct].list);
  INIT_LIST_HEAD(&dpo_dust[dust_ct].animator_list);
  list_add(&dpo_dust[dust_ct].list, &display_list);
  add_animator(&dpo_dust[dust_ct], ANIM_DUST);
  spr_set_pos(&dust_spr[dust_ct], x, y + 8);
  spr_show(&dust_spr[dust_ct]);

  dust_ct++;
}

void add_snow_storm()
{
  int8_t i, snow_ct;

  snow_ct = 0;

  for (i = 0; i < NUM_SNOW_SMALL; i++) {
    add_snow(dpo, snow_ct, PATRN_SNOW_SMALL);
    add_animator(dpo, ANIM_SNOW);
    dpo++; snow_ct++;
  }

  for (i = 0; i < NUM_SNOW_BIG; i++) {
    add_snow(dpo, snow_ct, PATRN_SNOW_BIG);
    add_animator(dpo, ANIM_SNOW);
    dpo++; snow_ct++;
  }
}

void show_intro()
{
  uint8_t fadein;
  int16_t i;
  bool done;

  const uint8_t title[28] =
    {58, 59, 60, 61, 62, 63, 64,
     74, 75, 76, 77, 78, 79, 80,
     90, 91, 92, 93, 94,95, 96,
     106, 107, 108, 109, 110, 111, 112};

  for(i = 0; i < 512; i++)
    scr_buffer[i] = 58; // empty square

  blit_map_tilebuffer(scr_buffer, &tiles_bs, 0);
  blit_map_tilebuffer_rect(title, &tiles_bs, 0, 9, 4, 14, 8);

  INIT_LIST_HEAD(&display_list);

  dpo_ct = 0;
  dpo = display_object;

  add_snow_storm();

  blit_font_vprintf(&font_bs, 14, 15, 0, "X+C");
  blit_font_vprintf(&font_bs, 10, 19, 0, "MATT THORSON");
  blit_font_vprintf(&font_bs, 11, 21, 0, "NOEL BERRY");
  blit_font_vprintf(&font_bs, 7, 23, 0, "MSX BY RETRODELUXE");

  vdp_screen_enable();

  done = false;
  fadein = 0;
  do {
    sys_irq_enable();
    sys_wait_vsync();
    spr_refresh();
    reftick = sys_get_ticks();

    trigger_a = sys_get_trigger(0) | sys_get_trigger(1);
    trigger_b = sys_get_trigger(3);
    animate_all();

    sys_irq_enable();

    if (trigger_a || trigger_b) fadein = 1;
    if (fadein) {
      if (fadein == 1 || fadein == 5) {
        for (i = 2; i < 16; i++) {
          palette[i].r = 7;
          palette[i].g = 7;
          palette[i].b = 7;
        }
        vdp_set_palette(palette);
      } else if (fadein == 3 || fadein == 7) {
        init_pal();
        vdp_set_palette(palette);
      } else if (fadein > 9) {
        for (i = 2; i < 16; i++) {
          if (palette[i].r > 0) palette[i].r--;
          if (palette[i].g > 0) palette[i].g--;
          if (palette[i].b > 0) palette[i].b--;
        }
        vdp_set_palette(palette);
        if (fadein > 20) {
          done = true;
        }
      }
      fadein++;
    }
  } while (!done);
}

void load_room(uint8_t x, uint8_t y)
{
  uint8_t tx, ty, tile, tile2x = 0;
  uint8_t *dst = scr_buffer;
  uint8_t *dst_col = col_buffer;
  uint16_t i = (x % 8 + y * 8) * 256; // 8192 - 256

  dust_ct = 0;

  init_pal();
  vdp_set_palette(palette);

  // need to clear the sprites from the intro....
  spr_clear();

  INIT_LIST_HEAD(&display_list);

  dpo_ct = 0;
  dpo = display_object;

  add_fast_snow_storm(dpo);

  for (ty = 0; ty < 16; ty++) {
    for (tx = 0; tx < 16; tx++) {
      tile = map_data[i];

      switch (tile) {
        case TYPE_VERTICAL_SPIKES:
        case TYPE_FAKE_WALL:
        case TYPE_MESSAGE:
        case TYPE_BIG_CHEST:
        case TYPE_BIG_CHEST+1:
        case TYPE_KEY:
        case TYPE_CHEST:
        case TYPE_FRUIT:
        case TYPE_FLY_FRUIT:
          break;
        case TYPE_PLAYER_SPAWN:
          log_e("add player %d %d\n", tx, ty);
          add_player(tx, ty);
        case TYPE_FALL_FLOOR:
        case TYPE_SPRING:
        case TYPE_BALLOON:
        case TYPE_FLAG:
        case TYPE_PLATFORM_A:
        case TYPE_PLATFORM_B:
          //tile = 1;
        default:
            tile2x = tile - 15;
            *dst++ = tile2x;
            *dst_col = tile2x;
            *(dst_col+1) = tile2x;
            *(dst_col+32) = tile2x;
            *(dst_col+33) = tile2x;
      }
      dst_col+=2;
      i++;
    }
    dst_col+=32;
  }



  list_for_each(elem, &display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
      spr_show(dpo->spr);
    } else if (dpo->type == DISP_OBJECT_TILE && dpo->visible) {
      //tile_object_show(dpo->tob, scr_buffer + 256, false);
    }
  }

  blit_map_tilebuffer(scr_buffer, &tiles_bs, 0);
  vdp_screen_enable();
}
