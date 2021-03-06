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
#include "ascii8.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"

#pragma CODE_PAGE 3

TileCollisionHandler handler[MAX_HANDLERS];

struct game_state_t game_state;

void init_handlers();

void init_game_state() {

  init_handlers();

  sys_memset(&game_state, 0, sizeof(game_state));

  game_state.jean_x = 100;
  game_state.jean_y = 192 - 64;
  game_state.jean_state = STATE_IDLE;
  game_state.jean_collision_state = 0;
  game_state.jean_anim_state = JANE_STATE_LEFT;
  game_state.room = ROOM_FOREST;

  game_state.checkpoint_x = game_state.jean_x;
  game_state.checkpoint_y = game_state.jean_y;
  game_state.checkpoint_room = game_state.room;

  game_state.change_room = false;
  game_state.live_cnt = GAME_MAX_LIVES;
  game_state.cross_cnt = 0;
  game_state.final_fight = false;
  game_state.templar_ct = 0;
  game_state.death = false;
  game_state.templar_delay = 0;
  game_state.show_parchment = 0;
  game_state.cross_switch = false;
  game_state.cup_picked_up = false;
  game_state.red_parchment = false;
  game_state.start_ending_seq = false;
  game_state.start_bonfire_seq = false;
  game_state.final_animation = false;
  game_state.teletransport = false;
  game_state.door_trigger = false;

  // debug helpers
  // game_state.bell = true;
  // game_state.toggle[0] = 1;
  // game_state.toggle[1] = 1;
  // game_state.toggle[2] = 1;
  // game_state.cross_switch = true;
  // game_state.cross_cnt = 12;
}

void handle_death() {
  game_state.jean_x = game_state.checkpoint_x;
  game_state.jean_y = game_state.checkpoint_y;
  game_state.jean_state = STATE_IDLE;
  game_state.room = game_state.checkpoint_room;
  game_state.death = false;
}

void null_handler(DisplayObject *dpo, uint8_t data) {
  unused(dpo);
  unused(data);
  // empty handler
}

void pickup_heart(DisplayObject *dpo, uint8_t data) {
  /** we may have multiple calls due to multiple colliding tiles,
      ensure no double counting happens **/
  if (game_state.hearth[data] == 0) {
    game_state.hearth[data] = 1;
    game_state.live_cnt++;
    game_state.refresh_score = true;
    remove_tileobject(dpo);
    sfx_play_effect(SFX_PICKUP_ITEM, 0);
  }
}

void pickup_scroll(DisplayObject *dpo, uint8_t data) {
  game_state.scroll[data] = 1;
  remove_tileobject(dpo);
  sfx_play_effect(SFX_PICKUP_ITEM, 0);
  game_state.show_parchment = data;
}

void pickup_red_scroll(DisplayObject *dpo, uint8_t data) {
  remove_tileobject(dpo);
  sfx_play_effect(SFX_PICKUP_ITEM, 0);
  game_state.show_parchment = data;
  game_state.start_ending_seq = true;
}

void pickup_cross(DisplayObject *dpo, uint8_t data) {
  /** we may have multiple calls due to multiple colliding tiles,
      ensure no double counting happens **/
  if (game_state.cross[data] == 0) {
    game_state.cross[data] = 1;
    game_state.cross_cnt++;
    game_state.refresh_score = true;
    remove_tileobject(dpo->parent);
    remove_tileobject(dpo);
    sfx_play_effect(SFX_PICKUP_ITEM, 0);
  }
}

void checkpoint_handler(DisplayObject *dpo, uint8_t data) {
  /** clear all other checkpoints */
  sys_memset(&game_state.checkpoint[0], 0, 8);

  /** last checkpoint is the only active one */
  game_state.checkpoint[data] = 1;
  game_state.checkpoint_x = dpo->xpos;
  game_state.checkpoint_y = dpo->ypos - 8;
  game_state.checkpoint_room = game_state.room;
  dpo->tob->frame = 1;
  phys_clear_colliding_tile_object(dpo);
  update_tileobject(dpo);
  sfx_play_effect(SFX_PICKUP_ITEM, 0);
}

void toggle_handler(DisplayObject *dpo, uint8_t data) {
  game_state.toggle[data] = 1;
  dpo->tob->frame = 1;
  update_tileobject(dpo);
  phys_clear_colliding_tile_object(dpo);
  sfx_play_effect(SFX_SWITCH, 0);
}

void bell_handler(DisplayObject *dpo, uint8_t data) {
  unused(data);
  game_state.bell = true;
  dpo->tob->frame = 1;
  update_tileobject(dpo);
  phys_clear_colliding_tile_object(dpo);
  dpo->parent->spr->frame = 1;
  spr_update(dpo->parent->spr);
  sfx_play_effect(SFX_SWITCH, 0);
}

void crosswitch_handler(DisplayObject *dpo, uint8_t data) {
  unused(data);
  /** switch is only active once every time it appears **/
  phys_clear_colliding_tile_object(dpo);
  if (game_state.cross_switch) {
    dpo->tob->frame = 0;
    dpo->parent->spr->frame = 0;
    game_state.cross_switch = false;
  } else {
    dpo->tob->frame = 1;
    dpo->parent->spr->frame = 1;
    game_state.cross_switch = true;
  }
  update_tileobject(dpo);
  spr_update(dpo->parent->spr);
  sfx_play_effect(SFX_SWITCH, 0);
}

void trigger_handler(DisplayObject *dpo, uint8_t data) {
  unused(dpo);
  if (data == TRIGGER_ENTRANCE_DOOR) {
    game_state.door_trigger = true;
  }
}

void deadly_tile_handler(DisplayObject *dpo, uint8_t data) {
  unused(dpo);
  unused(data);
  if (dpo_jean.state != STATE_COLLISION && dpo_jean.state != STATE_DEATH) {
    dpo_jean.state = STATE_COLLISION;
  }
}

void cup_handler(DisplayObject *dpo, uint8_t data) {
  unused(data);
  remove_tileobject(dpo);
  sfx_play_effect(SFX_PICKUP_ITEM, 0);
  game_state.cup_picked_up = true;
}

void teletransport(DisplayObject *dpo, uint8_t data) {
  unused(data);
  if (game_state.teletransport)
    return;

  // need to change jeans position and also room
  // but only trigger the change if jean is fully inside the portal
  if (game_state.room == ROOM_CAVE_DRAGON) {
    if (stick == STICK_UP) {
      game_state.room = ROOM_HIDDEN_RIVER;
      game_state.jean_x = 167;
      game_state.jean_y = 108;
      game_state.jean_state = STATE_IDLE;
      game_state.teletransport = true;
      phys_clear_colliding_tile_object(dpo);
      sfx_play_effect(SFX_PORTAL, 0);
    }
  } else if (game_state.room == ROOM_HIDDEN_RIVER) {
    if (stick == STICK_UP) {
      game_state.room = ROOM_CAVE_DRAGON;
      game_state.jean_x = 180;
      game_state.jean_y = 124;
      game_state.jean_state = STATE_IDLE;
      game_state.teletransport = true;
      phys_clear_colliding_tile_object(dpo);
      sfx_play_effect(SFX_PORTAL, 0);
    }
  }
}

void init_handlers() {
  uint8_t i;

  ascii8_get_page(pickup_heart);
  for (i = 0; i < MAX_HANDLERS; i++) {
    handler[i].page = ascii8_page;
  }

  handler[PICKUP_HEART].handler = pickup_heart;
  handler[PICKUP_SCROLL].handler = pickup_scroll;
  handler[PICKUP_RED_SCROLL].handler = pickup_red_scroll;
  handler[PICKUP_CROSS].handler = pickup_cross;
  handler[CHECKPOINT].handler = checkpoint_handler;
  handler[TOGGLE].handler = toggle_handler;
  handler[BELL].handler = bell_handler;
  handler[CROSSWITCH].handler = crosswitch_handler;
  handler[TRIGGER].handler = trigger_handler;
  handler[DEADLY_TILE].handler = deadly_tile_handler;
  handler[CUP].handler = cup_handler;
  handler[TELETRANSPORT].handler = teletransport;
}
