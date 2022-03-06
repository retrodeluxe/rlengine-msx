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
#include "scene.h"
#include "celeste.h"

const uint8_t sine[MAX_SINE] = {
  32,34,35,37,38,40,41,43,
  44,46,47,48,50,51,52,53,
  55,56,57,58,59,59,60,61,
  62,62,63,63,63,64,64,64,
  64,64,64,64,63,63,63,62,
  62,61,60,59,59,58,57,56,
  55,53,52,51,50,48,47,46,
  44,43,41,40,38,37,35,34,
  32,30,29,27,26,24,23,21,
  20,18,17,16,14,13,12,11,
  9,8,7,6,5,5,4,3,
  2,2,1,1,1,0,0,0,
  0,0,0,0,1,1,1,2,
  2,3,4,5,5,6,7,8,
  9,11,12,13,14,16,17,18,
  20,21,23,24,26,27,29,30
};

Animator animators[MAX_ANIMATORS];
Animator *anim;

#pragma CODE_PAGE 4

void add_animator(DisplayObject *dpo, enum anim_t animidx) {
  list_add(&animators[animidx].list, &dpo->animator_list);
}

void anim_snow(DisplayObject *obj)
{
  SpriteDef *sp = obj->spr;
  uint8_t amp;

  obj->xpos += obj->aux;
  obj->state += obj->aux2;
  if (obj->state > MAX_SINE) obj->state = 0;
  amp = sine[obj->state];
  spr_set_pos(sp, obj->xpos, obj->ypos + amp);
  spr_update(sp);
}

void anim_fast_snow(DisplayObject *obj)
{
  SpriteDef *sp;
  int16_t y;
  uint8_t amp, i, x, st;

  if (obj->state++ > 1) {
    for (i = 0; i< NUM_SNOW_SMALL + NUM_SNOW_BIG; i++) {
      sp = &snow_spr[i];
      x = (sp->planes[0]).x;
      y = snow_y[i];
      x += snow_aux[i];
      st = snow_status[i];
      st += snow_aux2[i];
      if (st > MAX_SINE) snow_status[i] = 0; else snow_status[i] = st;
      amp = sine[st];
      spr_set_pos(sp, x, y + amp);
      spr_update(sp);
    }
    obj->state = 0;
  }
}


void anim_dust(DisplayObject *obj)
{
  SpriteDef *sp = obj->spr;
  sp->anim_ctr++;
  obj->xpos++;
  if (sp->anim_ctr > 3) {
    sp->frame++;
    sp->anim_ctr = 0;
  }
  if (sp->frame > 2) {
      spr_hide(obj->spr);
      list_del(&obj->list);
      dust_ct--;
  } else {
    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
  }
}

void anim_shake(DisplayObject *obj) {
  int8_t dx = 0, dy = 0;

  switch (obj->aux2++) {
    case 2:
      dy = 1;
      break;
    case 3:
      dy = 0;
      break;
    case 4:
      dx = 1;
      break;
    case 5:
      dx = 0;
      break;
    case 6:
      list_del(&animators[ANIM_SHAKE].list);
      break;
  }
  vdp_display_adjust(dx, dy);
}

#define DX 2
#define DY_JUMP 4
#define DX_JUMP 4
#define DY_FALL 4

/**
 * long jump from down the screen until y target coordinate
 * then fall
 */
void anim_player_spawn(DisplayObject *obj) {

  SpriteDef *sp = obj->spr;
  SpritePattern *ps = sp->pattern_set;

  if (obj->state == 0 && obj->ypos > obj->aux - 12) {
    obj->ypos -= 6;
  } else {
    obj->state = 1;
    obj->ypos += 2;
  }

  sp->state = PLAYER_ANIM_RIGHT;
  sp->anim_ctr++;
  if (sp->anim_ctr > sp->anim_ctr_treshold) {
    sp->frame++;
    sp->anim_ctr = 0;
  }
  if (sp->frame > ps->state_steps[sp->state] - 1)
    sp->frame = 0;

  spr_set_pos(sp, obj->xpos, obj->ypos);
  spr_update(sp);
  if (obj->state == 1 && (obj->ypos > obj->aux)) {
    obj->state = 0;
    obj->aux2 = 1; // enable shake
    list_del(&obj->animator_list);
    add_animator(obj, ANIM_PLAYER);
  }
}


void anim_player(DisplayObject *obj) {
    int8_t dx, dy;
    static int16_t jmp =0, jmp_ct = 0;
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
            jmp_ct = 0;
            add_dust(obj->xpos, obj->ypos);
          }
          if (trigger_b) {
            log_e("long jump\n");
          }
          if (!is_colliding_down(obj)) {
            obj->state = STATE_FALLING;
          }
          break;
        case STATE_JUMPING:
          dy = jmp;
          if (jmp++ == 0) {
            obj->state = STATE_FALLING;
          }
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
          if (jmp++ > DY_FALL)
              jmp = DY_FALL;
          if (stick == STICK_LEFT) {
            dx = -DX_JUMP;
          } else if (stick == STICK_RIGHT) {
            dx = DX_JUMP;
          }
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
            add_dust(obj->xpos, obj->ypos);
            if (obj->aux2 == 1) {
              obj->aux2 == 2;
              add_animator(obj, ANIM_SHAKE);
            }
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

    //log_e("pos x %d y %d\n", obj->xpos, obj->ypos);
    // this is broken
    phys_detect_tile_collisions(obj, col_buffer, dx, dy, false, true);

  //  log_e("anim dx %d dy %d\n", dx, dy);
    //log_e("pos x %d y %d\n", obj->xpos, obj->ypos);

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
        if (obj->xpos < 0) obj->xpos = 0;
    }

    if ((dy > 0 && !is_colliding_down(obj)) ||
      (dy < 0 && !is_colliding_up(obj))) {
        obj->ypos += dy;
    }

    //log_e("pos x %d y %d\n", obj->xpos, obj->ypos);

    spr_set_pos(sp, obj->xpos, obj->ypos);
    spr_update(sp);
}

void init_animators() {
  uint8_t i;

  for (i = 0; i < MAX_ANIMATORS; i++) {
    animators[i].page = 4;
  }

  animators[ANIM_PLAYER].run = anim_player;
  animators[ANIM_PLAYER_SPAWN].run = anim_player_spawn;
  animators[ANIM_SNOW].run = anim_snow;
  animators[ANIM_FAST_SNOW].run = anim_fast_snow;
  animators[ANIM_DUST].run = anim_dust;
  animators[ANIM_SHAKE].run = anim_shake;
}
