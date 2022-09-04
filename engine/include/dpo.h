/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _MSX_DISP_LIST_H_
#define _MSX_DISP_LIST_H_

#include "list.h"
#include "sprite.h"
#include "tile.h"
#include "ascii8.h"

#define DPO_SPRITE 1
#define DPO_TILE 2
#define DPO_COMBO 3

/**
 * An object to be displayed as part of a display list.
 */
typedef struct DisplayObject DisplayObject;

/**
 * contents of a DisplayObject
 */
struct DisplayObject {

  /** DisplayObject type, see :c:enum:DisplayObjectType */
  uint8_t type;

  /** Maximum coordinate used for animation */
  uint8_t max;

  /** Minimum coordinate used in animation */
  uint8_t min;

  /** Speed of movement, used in animation */
  uint8_t speed;

  /** Color override, used to in animation */
  uint8_t color;

  /** Indicates if the object should be visible in screen */
  bool visible;

  /** Indicates if collision detection should be carried out for this object */
  bool check_collision;

  /** Animation state */
  uint8_t state;

  /** Auxiliary animation data */
  uint8_t aux;

  /** Auxiliary animation data */
  uint8_t aux2;

  /** Current screen position */
  int16_t xpos;

  /** Current screen position */
  int16_t ypos;

  /** Current collision state */
  uint8_t collision_state;

  /** Sprite definition */
  SpriteDef *spr;

  /** TileObject definition */
  TileObject *tob;

  /** Parent object, used for animation */
  DisplayObject *parent;

  List list;
  List animator_list;
};

/**
 * Contains an animator
 */
typedef struct Animator Animator;
struct Animator {
  List list;
  uint8_t page; // HACK: store animator page to allow switching
  void (*run)(DisplayObject *obj);
};

extern Animator *dpo_animators;

/**
 * Defines the animator with the given index
 *
 * :param anim_idx: index of the animator
 * :param anim: animator function
 */
#define dpo_define_animator(ANIMIDX, SYMBOL)  \
  ascii8_get_page(SYMBOL);                 \
  dpo_animators[ANIMIDX].page = ascii8_page;  \
  dpo_animators[ANIMIDX].run = SYMBOL;


extern void dpo_init();
extern void dpo_clear();
extern void dpo_init_animators(uint8_t n_animators);
extern void dpo_display_list_add(DisplayObject *dpo);
extern void dpo_add_animator(DisplayObject *dpo, uint8_t animidx);
extern void dpo_show_all(uint8_t *scr_buffer) __nonbanked;
extern void dpo_animate_all() __nonbanked;

extern DisplayObject *dpo_new();

#endif
