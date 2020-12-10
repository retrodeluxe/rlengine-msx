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

typedef enum {
  SPR_SIZE_8x8 = 1,
  SPR_SIZE_16x16 = 4,
  SPR_SIZE_16x32 = 8,
  SPR_SIZE_32x16 = 16,
  SPR_SIZE_32x32 = 32
} SpriteSize;

/* max number of patterns to be held in memory */
#define SPR_PATRN_MAX 48

/* max number of states per sprites */
#define SPR_STATES_MAX 8

/* predefined states for simple amimation */
enum spr_state {
  SPR_STATE_LEFT,
  SPR_STATE_RIGHT,
  SPR_STATE_UP,
  SPR_STATE_DOWN
};

/**
 * spr_pattern_set:
 *		a set of sprite patterns plus data used for animation
 */
typedef struct SpritePattern SpritePattern;
struct SpritePattern {
  uint8_t pidx;
  bool allocated;
  uint8_t size;
  uint8_t n_planes;
  uint8_t n_states;
  uint8_t state_steps[SPR_STATES_MAX];
  uint8_t n_steps;
  uint8_t *patterns;
  uint8_t *colors;
  uint8_t colors2[24]; // FIXME: this requires some sort of dynamic allocation
};

/**
 * spr_sprite_def:
 *		copy of sprite attributes as in vram plus animation status
 */
typedef struct SpriteDef SpriteDef;
struct SpriteDef {
  uint8_t aidx;
  VdpSpriteAttr planes[6];
  SpritePattern *pattern_set;
  uint8_t cur_state;
  uint8_t cur_anim_step;
  uint8_t state_anim_ctr[SPR_STATES_MAX];
  uint8_t anim_ctr;
  uint8_t anim_ctr_treshold;
};

extern SpritePattern spr_pattern[SPR_PATRN_MAX];

#define SPR_DEFINE_PATTERN_SET(X, SIZE, PLANES, STATES, STEPS, PATTERNS)       \
  spr_pattern[(X)].size = (SIZE);                                              \
  spr_pattern[(X)].n_planes = (PLANES);                                        \
  sys_memcpy(spr_pattern[(X)].state_steps, (STEPS), (STATES));                 \
  spr_pattern[(X)].n_states = (STATES);                                        \
  spr_pattern[(X)].allocated = false;                                          \
  spr_pattern[(X)].patterns = (PATTERNS);                                      \
  spr_pattern[(X)].colors = PATTERNS##_color

extern void spr_init();
extern void spr_clear();
extern void spr_init_sprite(SpriteDef *sp, uint8_t patrn_idx);
extern uint8_t spr_valloc_pattern_set(uint8_t patrn_idx);
extern void spr_vfree_pattern_set(uint8_t patrn_idx);
extern void spr_set_pos(SpriteDef *sp, int16_t xp, int16_t yp) __nonbanked;
extern void spr_set_plane_colors(SpriteDef *sp, uint8_t *colors) __nonbanked;
extern uint8_t spr_show(SpriteDef *sp) __nonbanked;
extern void spr_update(SpriteDef *sp) __nonbanked;
extern void spr_hide(SpriteDef *sp) __nonbanked;
extern void spr_animate(SpriteDef *sp, signed char dx,
                        signed char dy) __nonbanked;
extern bool spr_is_allocated(uint8_t patrn_idx);
extern void spr_refresh(void);
#endif
