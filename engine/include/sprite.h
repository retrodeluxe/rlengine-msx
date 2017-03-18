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

/**
 * spr_pattern_set:
 *		a set of sprite patterns plus data used for animation
 */
struct spr_pattern_set {
	uint8_t pidx;
	bool allocated;
	uint8_t size;
	uint8_t n_planes;
	uint8_t n_dirs;
	uint8_t n_anim_steps;
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
	uint8_t cur_dir;
	uint8_t cur_anim_step;
	uint8_t anim_ctr;
	uint8_t anim_ctr_treshold;
};


struct spr_delta_pos {
	char dx;
	char dy;
};

/**
 * helper macros for sprite definition from generated data
 */
#define SPR_DEFINE_PATTERN_SET(X, SIZE, PLANES, DIRS, STEPS, PATTERNS) 	(X).size = (SIZE);\
									(X).n_planes = (PLANES);\
									(X).n_anim_steps = (STEPS);\
									(X).n_dirs = (DIRS); \
									(X).allocated = false; \
									(X).patterns = (PATTERNS); \
									(X).colors = PATTERNS ## _color

#define SPR_DEFINE_SPRITE(X, PATTERN_SET, ANIM_TRH, COLORS)		(X).pattern_set = (PATTERN_SET);\
									(X).cur_anim_step = 0; \
									(X).cur_dir = 0;\
									(X).anim_ctr_treshold = (ANIM_TRH);\
									(X).anim_ctr = 0;\
									spr_set_plane_colors(&(X),(COLORS))


extern void spr_init();
extern void spr_init_sprite(struct spr_sprite_def *sp, struct spr_pattern_set *ps);
extern uint8_t spr_valloc_pattern_set(struct spr_pattern_set *ps);
extern void spr_vfree_pattern_set(struct spr_pattern_set *ps);
extern void spr_set_pos(struct spr_sprite_def *sp, uint8_t x, uint8_t y);
extern void spr_set_plane_colors(struct spr_sprite_def *sp, uint8_t * colors);
extern uint8_t spr_show(struct spr_sprite_def *sp);
extern void spr_update(struct spr_sprite_def *sp);
extern void spr_hide(struct spr_sprite_def *sp);
extern void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy,
		     char collision);

#endif
