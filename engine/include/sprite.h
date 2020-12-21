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

#ifndef _MSX_H_SPRITE
#define _MSX_H_SPRITE

#include "vdp.h"

/**
 * Size of a Sprite
 */
typedef enum {
  /**
   * 16x16 pixel sprite, each frame is a superposition of 16x16 hardware sprites.
   */
  SPR_SIZE_16x16 = 4,

  /**
   * 16x32 pixel sprite, composed by two 16x16 sprites.
   */
  SPR_SIZE_16x32 = 8,

  /**
   * 32x16 pixel sprite, composed by two 16x16 sprites.
   */
  SPR_SIZE_32x16 = 16,

  /**
   * 32x32 pixel sprite, composed by four 16x16 sprites.
   */
  SPR_SIZE_32x32 = 32
} SpriteSize;

/** Max number of patterns that are held in RAM */
#define SPR_PATRN_MAX 48

/** Max number of states per sprites */
#define SPR_STATES_MAX 8

/**
 * Set of predefined sprite states that can be used with :c:func:`spr_animate`
 */
enum spr_state {

  /**
   * Moving left
   */
  SPR_STATE_LEFT,

  /**
   * Moving right
   */
  SPR_STATE_RIGHT,

  /**
   * Moving up
   */
  SPR_STATE_UP,

  /**
   * Moving down
   */
  SPR_STATE_DOWN
};

/**
 * Defines a sprite pattern
 */
typedef struct SpritePattern SpritePattern;

/**
 * Contains a software sprite pattern composed of many individual
 * hardware sprite patterns, along with metadata to describe the structure.
 */
struct SpritePattern {
  uint8_t pidx;

  /** Indicates if the pattern has been allocated in VRAM */
  bool allocated;

  /** Size, see :c:enum:`SpriteSize` */
  uint8_t size;

  /** Number of planes (colors) */
  uint8_t n_planes;

  /** Number of states */
  uint8_t n_states;

  /** Animation steps per state */
  uint8_t state_steps[SPR_STATES_MAX];

  /** Total number of animation steps */
  uint8_t n_steps;

  /** Raw hardware sprite pattern data */
  uint8_t *patterns;

  /** Raw hardware sprite color data */
  uint8_t *colors;

  uint8_t colors2[24]; // FIXME: this requires some sort of dynamic allocation
};

/**
 * Contains a software sprite definition
 */
typedef struct SpriteDef SpriteDef;

/**
 * Contains sprite attributes like position and color, along with the
 * current animation state.
 */
struct SpriteDef {
  uint8_t aidx;
  VdpSpriteAttr planes[6];
  SpritePattern *pattern_set;

  /** current sprite state */
  uint8_t cur_state;

  /** current animation step within the current state */
  uint8_t cur_anim_step;

  uint8_t state_anim_ctr[SPR_STATES_MAX];
  uint8_t anim_ctr;
  uint8_t anim_ctr_treshold;
};

extern SpritePattern spr_pattern[SPR_PATRN_MAX];

/**
 * Helper macro for filling in a SpritePattern structure using resource data
 *
 * :param INDEX: Pattern index (value between 0 and 47)
 * :param SIZE: a SpriteSize
 * :param PLANES: number of planes
 * :param STATES: number of states
 * :param STEPS: array containing the number of animation steps per state
 * :param PATTERNS: patterns binary data
 */
#define SPR_DEFINE_PATTERN_SET(INDEX, SIZE, PLANES, STATES, STEPS, PATTERNS)       \
  assert((INDEX) < SPR_PATRN_MAX, "Max pattern index should be below 48");         \
  spr_pattern[(INDEX)].size = (SIZE);                                              \
  spr_pattern[(INDEX)].n_planes = (PLANES);                                        \
  sys_memcpy(spr_pattern[(INDEX)].state_steps, (STEPS), (STATES));                 \
  spr_pattern[(INDEX)].n_states = (STATES);                                        \
  spr_pattern[(INDEX)].allocated = false;                                          \
  spr_pattern[(INDEX)].patterns = (PATTERNS);                                      \
  spr_pattern[(INDEX)].colors = PATTERNS##_color;                                  \
  if (SIZE > SPR_SIZE_32x16)                                                       \
    assert(PLANES < 2, "Max 1 plane allowed in 32x32 sprites");                    \
  if (SIZE > SPR_SIZE_16x16)                                                       \
    assert(PLANES < 3, "Max 2 planes allowed in 16x32 and 32x16 sprites");          \
  assert(PLANES < 4, "Max 3 planes allowed in 16x16 sprites")

extern void spr_init();
extern void spr_clear();
extern void spr_init_sprite(SpriteDef *sp, uint8_t patrn_idx);
extern bool spr_valloc_pattern_set(uint8_t patrn_idx);
extern void spr_vfree_pattern_set(uint8_t patrn_idx);
extern void spr_set_pos(SpriteDef *sp, int16_t xp, int16_t yp) __nonbanked;
extern void spr_set_plane_colors(SpriteDef *sp, uint8_t *colors) __nonbanked;
extern bool spr_show(SpriteDef *sp) __nonbanked;
extern void spr_update(SpriteDef *sp) __nonbanked;
extern void spr_hide(SpriteDef *sp) __nonbanked;
extern void spr_animate(SpriteDef *sp, int8_t dx, int8_t dy) __nonbanked;
extern void spr_refresh(void);
#endif
