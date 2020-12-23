#include "dpo.h"
#include "list.h"
#include "log.h"
#include "map.h"
#include "msx.h"
#include "phys.h"
#include "sfx.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"

#include "anim.h"
#include "banks.h"
#include "logic.h"
#include "scene.h"

#pragma CODE_PAGE 7

extern void add_bullet(uint8_t xpos, uint8_t ypos, uint8_t patrn_id,
                       uint8_t anim_id, uint8_t state, uint8_t dir,
                       uint8_t speed, DisplayObject *parent);
extern void add_tob_bullet(uint8_t xpos, uint8_t ypos, uint8_t tileidx,
                           uint8_t anim_id, uint8_t state, uint8_t dir,
                           uint8_t speed, DisplayObject *parent);

extern DisplayObject dpo_tob_bullet[SCENE_MAX_TOB_BULLET];
extern DisplayObject dpo_bullet[SCENE_MAX_BULLET];
extern TileObject bullet_tob[SCENE_MAX_TOB_BULLET];
extern TileSet tileset[TILE_MAX];
extern SpriteDef bullet_sprites[SCENE_MAX_BULLET];

void clear_bullets() {
  uint8_t idx;
  for (idx = 0; idx < SCENE_MAX_BULLET; idx++) {
    if (dpo_bullet[idx].state != 255) {
      spr_hide(&bullet_sprites[idx]);
      if (dpo_bullet[idx].state == 1)
        list_del(&dpo_bullet[idx].list);
    }
  }
}

void add_explosion(uint8_t xpos, uint8_t ypos, uint8_t anim_id) {
  uint8_t idx = 0;

  ascii8_set_data(PAGE_RAW_TILES);
  tile_set_vfree(&tileset[TILE_SATAN]);
  tile_set_valloc(&tileset[TILE_EXPLOSION]); // explosion

  bullet_tob[idx].x = xpos;
  bullet_tob[idx].y = ypos;
  bullet_tob[idx].cur_dir = 0;
  bullet_tob[idx].cur_anim_step = 0;
  bullet_tob[idx].ts = &tileset[TILE_EXPLOSION];
  bullet_tob[idx].idx = 0;

  dpo_tob_bullet[idx].type = DISP_OBJECT_TILE;
  dpo_tob_bullet[idx].tob = &bullet_tob[0];
  dpo_tob_bullet[idx].xpos = xpos;
  dpo_tob_bullet[idx].ypos = ypos;
  dpo_tob_bullet[idx].visible = true;
  dpo_tob_bullet[idx].state = 0;
  INIT_LIST_HEAD(&dpo_tob_bullet[0].list);
  list_add(&dpo_tob_bullet[0].list, &display_list);
  INIT_LIST_HEAD(&dpo_tob_bullet[0].animator_list);
  add_animator(&dpo_tob_bullet[0], anim_id);
  tile_object_show(dpo_tob_bullet[0].tob, scr_tile_buffer, true);
}

/**
 * Simplified animation for a Templar chasing Jean in intro screen
 */
void anim_intro_chase(DisplayObject *obj) {
  int8_t dx = 0, dy = 0;
  SpriteDef *sp = obj->spr;

  obj->aux++;
  switch (obj->state) {
  case STATE_MOVING_RIGHT:
    dx = 2;
    break;
  case STATE_OFF_SCREEN:
    if (obj->aux > 10) {
      obj->state = STATE_MOVING_RIGHT;
      obj->visible = true;
      spr_show(obj->spr);
    }
    return;
  case STATE_OFF_SCREEN_DELAY_1S:
    if (obj->aux > 19) {
      obj->state = STATE_OFF_SCREEN;
    }
    return;
  case STATE_OFF_SCREEN_DELAY_2S:
    if (obj->aux > 29) {
      obj->state = STATE_OFF_SCREEN;
    }
    return;
  }

  obj->xpos += dx;

  if (obj->visible) {
    spr_animate(sp, dx, dy);
    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
  }

  if (obj->xpos > 254) {
    spr_hide(sp);
    list_del(&obj->list);
  }
}

/**
 * Simplified Jean animation for intro screen
 */
void anim_intro_jean(DisplayObject *obj) {
  SpriteDef *sp = obj->spr;
  SpritePattern *ps = sp->pattern_set;

  obj->xpos += 2;
  sp->anim_ctr++;

  if (sp->anim_ctr > sp->anim_ctr_treshold) {
    sp->cur_anim_step++;
    sp->anim_ctr = 0;
  }
  if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
    sp->cur_anim_step = 0;

  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);

  if (obj->xpos > 254) {
    spr_hide(sp);
    list_del(&obj->list);
  }
}

/**
 * Animation Death Boss
 */
void anim_death(DisplayObject *obj) {
  SpriteDef *sp = obj->spr;
  int8_t dx = 0;

  dx = obj->speed;
  switch (obj->state) {
  case STATE_MOVING_LEFT:
    if (obj->xpos > obj->max) {
      obj->state = STATE_MOVING_RIGHT;
      dx *= -1;
      ;
    }
    break;
  case STATE_MOVING_RIGHT:
    dx *= -1;
    if (obj->xpos < obj->min) {
      obj->state = STATE_MOVING_LEFT;
      dx *= -1;
    }
    break;
  }
  // based on current animation state, throw bullet which is a scythe with
  // own animation
  if (sp->cur_anim_step == 2 && sp->anim_ctr == 0 && obj->aux2 < obj->aux) {
    obj->aux2++;
    add_bullet(obj->xpos, obj->ypos + 24, PATRN_SCYTHE, ANIM_SCYTHE, 0, 1, 1,
               obj);
  }

  obj->xpos += dx;
  spr_animate(sp, dx, 0);
  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);
}

/**
 * animation of scythe bullets, slowly falling with collision detection,
 *  bullets are deleted when reaching screen borders; shouldn't have more than
 *  3 active simultaneously
 */
void anim_scythe(DisplayObject *obj) {
  int8_t dx, dy;
  SpriteDef *sp = obj->spr;

  dx = obj->aux;
  dy = 1;

  if (obj->state == 0) {
    sp->anim_ctr_treshold = 1;
    obj->state = 1;
    if (obj->xpos > dpo_jean.xpos)
      obj->aux *= -1;
  }

  phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false, false);
  if (!is_colliding_down(obj)) {
    obj->ypos += dy;
  }
  obj->xpos += dx;

  if (obj->xpos < 4 || obj->xpos > 235 || obj->ypos > 150) {
    spr_hide(obj->spr);
    list_del(&obj->list);
    obj->state = 255;
    obj->parent->aux2--;
  } else {
    spr_animate(sp, dx, dy);
    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
  }
}

/**
 *  Hack to handle satan bullets as a bundle
 */
void add_satan_bullets(uint8_t xpos, uint8_t ypos) {
  uint8_t idx;

  idx = 0;
  for (idx = 0; idx < SCENE_MAX_BULLET; idx++) {
    if (dpo_bullet[idx].state == 255)
      break;
  }

  if (idx == SCENE_MAX_BULLET)
    return;

  // repeat 3 times
  spr_init_sprite(&bullet_sprites[idx], PATRN_BULLET);
  spr_init_sprite(&bullet_sprites[idx + 1], PATRN_BULLET);
  spr_init_sprite(&bullet_sprites[idx + 2], PATRN_BULLET);
  bullet_sprites[idx].cur_state = 0;
  bullet_sprites[idx].cur_anim_step = 0;
  bullet_sprites[idx + 1].cur_state = 0;
  bullet_sprites[idx + 1].cur_anim_step = 0;
  bullet_sprites[idx + 2].cur_state = 0;
  bullet_sprites[idx + 2].cur_anim_step = 0;
  spr_set_pos(&bullet_sprites[idx], xpos, ypos);
  spr_set_pos(&bullet_sprites[idx + 1], xpos, ypos + 8);
  spr_set_pos(&bullet_sprites[idx + 2], xpos, ypos + 16);
  spr_show(&bullet_sprites[idx]);
  spr_show(&bullet_sprites[idx + 1]);
  spr_show(&bullet_sprites[idx + 2]);
  // asign only the first one, store index in aux
  dpo_bullet[idx].type = DISP_OBJECT_SPRITE;
  dpo_bullet[idx].spr = &bullet_sprites[idx];
  dpo_bullet[idx].xpos = xpos;
  dpo_bullet[idx].ypos = ypos;
  dpo_bullet[idx].state = 1;
  dpo_bullet[idx + 1].state = 0;
  dpo_bullet[idx + 2].state = 0; // save 3 spots
  dpo_bullet[idx].collision_state = 0;
  dpo_bullet[idx].aux = idx;
  dpo_bullet[idx].aux2 = 7; // store sprite bullet state in 3 LSB

  // only one tob is added - animations happens for a sequence
  INIT_LIST_HEAD(&dpo_bullet[idx].list);
  INIT_LIST_HEAD(&dpo_bullet[idx].animator_list);
  add_animator(&dpo_bullet[idx], ANIM_SATAN_BULLETS);
  list_add(&dpo_bullet[idx].list, &display_list);
}

/**
 * Final Boss animation, big tile object that moves vertically and
 *  spits clusters of 3 bullets
 */
void anim_satan(DisplayObject *obj) {
  uint8_t x, ty;
  static uint8_t delay = 0;
  TileObject *to = obj->tob;
  uint16_t offset_bottom =
      to->x / 8 + to->y / 8 * 32 + (to->ts->frame_h - 1) * 32;
  uint16_t offset_top = to->x / 8 + to->y / 8 * 32;

  if (obj->aux2) {
    obj->aux2--;
    return;
  } else {
    obj->aux2 = 1; // reduce framerate
  }

  /** cup has been picked up **/
  if (game_state.cup_picked_up) {
    clear_bullets();
    phys_clear_colliding_tile_object(obj);
    tile_object_hide(obj->tob, scr_tile_buffer, true);
    list_del(&obj->list);
    add_explosion(obj->xpos, obj->ypos, ANIM_EXPLOSION);
    return;
  }

  if (obj->state == STATE_MOVING_UP) {
    if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->tob->cur_anim_step++;
    } else {
      for (x = 0; x < to->ts->frame_w; x++) {
        vdp_write(VRAM_BASE_NAME + offset_bottom + x, 0);
      }
      ty = obj->ypos / 8;
      ty--;
      obj->ypos = ty * 8;
      obj->tob->y = ty * 8;
      if (obj->aux++ > 3) {
        obj->tob->cur_anim_step = 0;
        add_satan_bullets(obj->xpos - 8, obj->ypos);
        obj->aux = 0;
      } else {
        obj->tob->cur_anim_step = 1;
      }
      tile_object_show(obj->tob, scr_tile_buffer, true);
    }
    if (obj->ypos < obj->min) {
      obj->state = STATE_MOVING_DOWN;
      obj->tob->cur_anim_step = 2;
    }
  } else if (obj->state == STATE_MOVING_DOWN) {
    if (obj->tob->cur_anim_step > 0) {
      if (obj->tob->cur_anim_step == 1 && obj->aux++ > 3) {
        obj->tob->cur_anim_step = 0;
        add_satan_bullets(obj->xpos - 8, obj->ypos);
        obj->aux = 0;
      } else {
        obj->tob->cur_anim_step--;
      }
      tile_object_show(obj->tob, scr_tile_buffer, true);
    } else {
      for (x = 0; x < to->ts->frame_w; x++) {
        vdp_write(VRAM_BASE_NAME + offset_top + x, 0);
      }
      ty = obj->ypos / 8;
      ty++;
      obj->ypos = ty * 8;
      obj->tob->y = ty * 8;
      obj->tob->cur_anim_step = 2;
      tile_object_show(obj->tob, scr_tile_buffer, true);
    }
    if (obj->ypos > obj->max) {
      obj->state = STATE_MOVING_UP;
      obj->tob->cur_anim_step = 1;
    }
  }
}

/**
 * Animation of final boss bullets, move linearly in diagonal directions
 *  no tile collisions; dissapear on wall.
 *
 * Hack to handle sets of 3 bullets in a bundle for performance
 */
void anim_satan_bullets(DisplayObject *obj) {
  SpriteDef *sp;
  uint8_t idx, i;
  int16_t x, y;
  int8_t dx, dy;

  dx = -4;
  dy = 0;
  idx = obj->aux;
  for (i = 0; i < 3; i++) {
    if (obj->aux2 & (1 << i)) {
      sp = &bullet_sprites[idx + i];

      if (i == 0) {
        dy = -1;
      } else if (i == 2) {
        dy = 1;
      } else {
        dy = 0;
      }

      x = sp->planes[0].x;
      y = sp->planes[0].y;

      x = x + dx;
      y = y + dy + 1; // correction y spr coords are offset -1

      if (x < 4 || y < 4) {
        spr_hide(sp);
        obj->aux2 &= ~(1 << i);
      } else {
        spr_animate(sp, dx, dy);
        spr_set_pos(sp, x, y);
        spr_update(sp);
      }
    }
  }
  if (obj->aux2 == 0) {
    list_del(&obj->list);
    dpo_bullet[idx].state = 255;
    dpo_bullet[idx + 1].state = 255;
    dpo_bullet[idx + 2].state = 255;
  }
}

/**
 * Animation for Dragon Flame, big tileobject with 2 frames
 *  it flames for alternating frames for ~4 seconds then stops and
 *  spits bullets (flames) on the ground in two directions
 */
void anim_dragon_flame(DisplayObject *obj) {
  obj->state++;
  if (obj->state < 30) {
    if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->tob->cur_anim_step++;
    } else {
      obj->tob->cur_anim_step = 0;
    }
  } else if (obj->state == 30) {
    // bullets are tileobjects... how to handle those
    add_tob_bullet(obj->xpos - 8, obj->ypos + 40, TILE_LAVA,
                   ANIM_DRAGON_BULLETS, 0, 0, 1, NULL);
    add_tob_bullet(obj->xpos + 24, obj->ypos + 40, TILE_LAVA,
                   ANIM_DRAGON_BULLETS, 0, 1, 1, NULL);
    tile_object_hide(obj->tob, scr_tile_buffer, true);
  } else if (obj->state == 60) {
    obj->state = 0;
  }
}

/**
 * Animation of dragon flame bullets, horizontal translation
 */
void anim_dragon_bullets(DisplayObject *obj) {
  uint8_t offset_before, offset_after;
  int16_t base_offset;
  int8_t dx;

  base_offset = (obj->ypos / 8) * 32;
  offset_before = obj->xpos / 8;

  dx = 1;
  if (obj->aux == 0) {
    dx = -1;
  }

  offset_after = (obj->xpos + dx) / 8;

  if (offset_before != offset_after) {
    vdp_write(VRAM_BASE_NAME + base_offset + offset_before + 1, 0);
    vdp_write(VRAM_BASE_NAME + base_offset + offset_before - 1, 0);
    *(scr_tile_buffer + base_offset + offset_before + 1) = 0;
    *(scr_tile_buffer + base_offset + offset_before - 1) = 0;
  }

  obj->xpos += dx;
  obj->tob->x += dx;

  if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
    tile_object_show(obj->tob, scr_tile_buffer, true);
    obj->tob->cur_anim_step++;
  } else {
    obj->tob->cur_anim_step = 0;
  }
  if (obj->xpos < 48 || obj->xpos > 250) {
    tile_object_hide(obj->tob, scr_tile_buffer, true);
    vdp_write(VRAM_BASE_NAME + base_offset + offset_before, 0);
    vdp_write(VRAM_BASE_NAME + base_offset + offset_before - 1, 0);
    *(scr_tile_buffer + base_offset + offset_before) = 0;
    *(scr_tile_buffer + base_offset + offset_before - 1) = 0;
    list_del(&obj->list);
    obj->state = 255;
  }
}

/**
 * Animation for priests hanging from tree, length of rope
 *  needs to be adjusted as priests move up and down.
 */
void anim_hanging_priest(DisplayObject *obj) {
  uint8_t ty;
  TileObject *to = obj->tob;
  uint16_t offset_bottom =
      to->x / 8 + to->y / 8 * 32 + (to->ts->frame_h - 1) * 32;
  uint16_t offset_top = to->x / 8 + to->y / 8 * 32;

  if (obj->state == STATE_MOVING_UP) {
    if (obj->tob->cur_anim_step > 0) {
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->tob->cur_anim_step--;
    } else {
      vdp_write(VRAM_BASE_NAME + offset_bottom, 0);
      vdp_write(VRAM_BASE_NAME + offset_bottom + 1, 0);
      *(scr_tile_buffer + offset_bottom) = 0;
      *(scr_tile_buffer + offset_bottom + 1) = 0;
      ty = obj->ypos / 8;
      ty--;
      obj->ypos = ty * 8;
      obj->tob->y = ty * 8;
      obj->tob->cur_anim_step = 3;
      tile_object_show(obj->tob, scr_tile_buffer, true);
    }
    if (obj->ypos < obj->min) {
      obj->state = STATE_MOVING_DOWN;
      obj->tob->cur_dir = 1;
      obj->tob->cur_anim_step = 0;
    }
  } else if (obj->state == STATE_MOVING_DOWN) {
    if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->tob->cur_anim_step++;
    } else {
      vdp_write(VRAM_BASE_NAME + offset_top, 0);
      vdp_write(VRAM_BASE_NAME + offset_top + 1, 180); // rope
      *(scr_tile_buffer + offset_top) = 0;
      *(scr_tile_buffer + offset_top + 1) = 100;
      ty = obj->ypos / 8;
      ty++;
      obj->ypos = ty * 8;
      obj->tob->y = ty * 8;
      obj->tob->cur_anim_step = 0;
      tile_object_show(obj->tob, scr_tile_buffer, true);
    }
    if (obj->ypos > obj->max) {
      obj->state = STATE_MOVING_UP;
      obj->tob->cur_dir = 0;
      obj->tob->cur_anim_step = 3;
    }
  }
}

void anim_explosion(DisplayObject *obj) {
  if (obj->state++ < 20) {
    if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->tob->cur_anim_step++;
    } else {
      obj->tob->cur_anim_step = 0;
    }
  } else {
    tile_object_hide(obj->tob, scr_tile_buffer, true);
    list_del(&obj->list);
    game_state.red_parchment = true;
  }
}

void anim_red_parchment(DisplayObject *obj) {
  if (game_state.red_parchment) {
    obj->visible = true;
    tile_object_show(obj->tob, scr_tile_buffer, true);
  }
}

void anim_evil_chamber(DisplayObject *obj) {
  int8_t dx = 0, dy = 0;
  SpriteDef *sp = obj->spr;

  obj->aux++;
  switch (obj->state) {
  case STATE_MOVING_RIGHT:
    dx = 2;
    break;
  case STATE_OFF_SCREEN:
    if (obj->aux > 10) {
      obj->state = STATE_MOVING_RIGHT;
      obj->visible = true;
      spr_show(obj->spr);
    }
    return;
  case STATE_OFF_SCREEN_DELAY_1S:
    if (obj->aux > 19) {
      obj->state = STATE_OFF_SCREEN;
    }
    return;
  case STATE_OFF_SCREEN_DELAY_2S:
    if (obj->aux > 29) {
      obj->state = STATE_OFF_SCREEN;
    }
    return;
  case STATE_OFF_SCREEN_DELAY_3S:
    if (obj->aux > 39) {
      obj->state = STATE_OFF_SCREEN;
    }
    return;
  }

  obj->xpos += dx;

  spr_animate(sp, dx, dy);
  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);

  if (obj->xpos > 140) {
    game_state.show_parchment = 8; // blue parchment
    game_state.start_bonfire_seq = true;
  }
}

/**
 * Animation for the bonfire ending sequence
 *  jean shakes left and right 3 times then dies.
 */
void anim_jean_bonfire(DisplayObject *obj) {
  // needs to last a bit longer....
  // and is not showing templar sprites...
  SpriteDef *sp = obj->spr;
  SpritePattern *ps = sp->pattern_set;

  if (obj->state == 0) {
    sp->cur_state = JANE_STATE_LEFT_JUMP;
    spr_update(sp);
  } else if (obj->state == 50) {
    sp->cur_state = JANE_STATE_RIGHT_JUMP;
    spr_update(sp);
  } else if (obj->state == 100) {
    sp->cur_state = JANE_STATE_LEFT_JUMP;
    spr_update(sp);
  } else if (obj->state == 150) {
    // doesn't sound because there isn't any music...
    sfx_play_effect(SFX_DEATH, 0);
  } else if (obj->state > 150 && obj->state < 170) {
    sp->cur_state = JANE_STATE_DEATH;
    sp->anim_ctr++;
    if (sp->anim_ctr > sp->anim_ctr_treshold) {
      sp->cur_anim_step++;
      sp->anim_ctr = 0;
    }
    if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
      sp->cur_anim_step = 0;

    spr_update(sp);
  } else if (obj->state == 170) {
    game_state.final_animation = true;
  }
  obj->state++;
}

/**
 * Animation for block crosses apearing sequentially on final boss
 */
// compiler issue, ct2 fails to initialize properly

void anim_block_crosses(DisplayObject *obj) {
  static uint8_t ct1 = 0;
  static uint8_t ct2 = 0;

  if (ct1++ > 40) {
    if (game_state.cross_cnt > 0) {
      ct2++;
      game_state.cross_cnt--;
      game_state.refresh_score = true;
    }
    ct1 = 0;
  }
  if (ct2 >= obj->aux) {
    if (obj->state == 0) {
      sfx_play_effect(SFX_SHOOT, 0);
      tile_object_show(obj->tob, scr_tile_buffer, true);
      obj->state++;
    }
    if (obj->state == 30) {
      if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
        tile_object_show(obj->tob, scr_tile_buffer, true);
        obj->tob->cur_anim_step++;
      } else {
        obj->tob->cur_anim_step = 0;
      }
      obj->state = 1;
    }
    obj->state++;
  }
}

/**
 * Animation for crosses with dynamic switch
 */
void anim_cross(DisplayObject *obj) {
  if (game_state.cross_switch) {
    if (obj->aux == 1)
      obj->visible = true;
    else
      obj->visible = false;
  } else if (!game_state.cross_switch) {
    if (obj->aux == 0)
      obj->visible = true;
    else
      obj->visible = false;
  }
  if (obj->visible) {
    if (obj->state++ == 5) {
      if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
        tile_object_show(obj->tob, scr_tile_buffer, true);
        obj->tob->cur_anim_step++;
      } else {
        obj->tob->cur_anim_step = 0;
      }
      obj->state = 0;
    }
  }
}

void anim_templar_bonfire(DisplayObject *obj) {
  SpriteDef *sp = obj->spr;
  spr_update(sp);
}

void anim_close_door_satan(DisplayObject *obj) {
  if (!obj->visible && dpo_jean.xpos > 20) {
    obj->visible = true;
    tile_object_show(obj->tob, scr_tile_buffer, true);
  }
}
