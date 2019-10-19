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

#define SPR_SIZE_8x8	1
#define SPR_SIZE_16x16	4
#define SPR_SIZE_16x32	8
//#define SPR_SIZE_32x16	8

#define spr_dir_lr 		1
#define spr_dir_lrud 	2
#define spr_dir_lrudc 	3
#define spr_dir_all 	4

#define SPR_SHOW_8x8 0
#define SPR_SHOW_16x16 1
#define SPR_ZOOM_OFF 0
#define SPR_ZOOM_ON 1

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
struct spr_pattern_set {
	uint8_t pidx;
	bool allocated;
	uint8_t size;
	uint8_t n_planes;
	uint8_t n_states;
	uint8_t state_steps[SPR_STATES_MAX];
	uint8_t n_steps;
	uint8_t *patterns;
	uint8_t *colors;
};

/**
 * spr_sprite_def:
 *		copy of sprite attributes as in vram plus animation status
 */
struct spr_sprite_def {
	uint8_t aidx;
	struct vdp_hw_sprite planes[6];
	struct spr_pattern_set *pattern_set;
	uint8_t cur_state;
	uint8_t cur_anim_step;
	uint8_t state_anim_ctr[SPR_STATES_MAX];
	uint8_t anim_ctr;
	uint8_t anim_ctr_treshold;
};


struct spr_delta_pos {
	char dx;
	char dy;
};

extern struct spr_pattern_set spr_pattern[SPR_PATRN_MAX];

/**
 * helper macros for sprite definition from generated data
 */
#define SPR_DEFINE_PATTERN_SET(X, SIZE, PLANES, STATES, STEPS, PATTERNS) 	spr_pattern[(X)].size = (SIZE);\
									spr_pattern[(X)].n_planes = (PLANES);\
									sys_memcpy(spr_pattern[(X)].state_steps, (STEPS), (STATES));\
									spr_pattern[(X)].n_states = (STATES); \
									spr_pattern[(X)].allocated = false; \
									spr_pattern[(X)].patterns = (PATTERNS); \
									spr_pattern[(X)].colors = PATTERNS ## _color

extern void spr_init();
//extern voud spr_define_pattern_set(uint8_t size, uint8_t planes, uint8_t state, uint8_t *steps, )
extern void spr_init_sprite(struct spr_sprite_def *sp, uint8_t patrn_idx);
extern uint8_t spr_valloc_pattern_set(uint8_t patrn_idx);
extern void spr_vfree_pattern_set(uint8_t patrn_idx);
extern void spr_set_pos(struct spr_sprite_def *sp, uint8_t x, uint8_t y);
extern void spr_set_plane_colors(struct spr_sprite_def *sp, uint8_t * colors);
extern uint8_t spr_show(struct spr_sprite_def *sp);
extern void spr_update(struct spr_sprite_def *sp);
extern void spr_hide(struct spr_sprite_def *sp);
extern void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy);

#endif
