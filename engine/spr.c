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

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "log.h"

#pragma CODE_PAGE 2

// vram sprite attribute allocation table
uint8_t spr_attr_valloc[vdp_hw_max_sprites];

// vram sprite pattern allocation table
uint8_t spr_patt_valloc[vdp_hw_max_patterns];

// spr pattern sets
struct spr_pattern_set spr_pattern[SPR_PATRN_MAX];

struct vdp_hw_sprite spr_attr[vdp_hw_max_sprites];
/**
 * spr_init: initialize vdp sprites and allocation tables
 */
void spr_init(void)
{
	spr_clear();
	sys_memset(spr_pattern, 0,  sizeof(struct spr_pattern_set) * SPR_PATRN_MAX);
}

void spr_refresh(void)
{
	vdp_memcpy(vdp_base_spatr_grp1, (uint8_t *)&spr_attr,
		sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites);
}

void spr_clear(void)
{
	uint8_t i;
	struct vdp_hw_sprite null_spr;

	vdp_init_hw_sprites(SPR_SHOW_16x16, SPR_ZOOM_OFF);

	null_spr.y = 193;
	null_spr.x = 0;
	null_spr.pattern = 0;
	null_spr.color = 128; // EC bit

	for (i = 0; i < vdp_hw_max_sprites; i++)
		sys_memcpy((uint8_t *)&spr_attr[i], (uint8_t *)&null_spr, sizeof(struct vdp_hw_sprite));

	sys_memset(spr_attr_valloc, 1, vdp_hw_max_sprites);
	sys_memset(spr_patt_valloc, 1, vdp_hw_max_patterns);

	// free pattern sets
	for(i = 0; i < SPR_PATRN_MAX; i++)
		spr_vfree_pattern_set(i);
}

void spr_init_sprite(struct spr_sprite_def *sp, uint8_t patrn_idx)
{
	sp->pattern_set = &spr_pattern[patrn_idx];
	sp->cur_anim_step = 0;
	sp->cur_state = 0;
	sp->anim_ctr_treshold = 5;
	sp->anim_ctr = 0;
	spr_set_plane_colors(sp, spr_pattern[patrn_idx].colors);
}

/**
 * spr_valloc_pattern_set:
 *		finds a gap to allocate a pattern set
 */
uint8_t spr_valloc_pattern_set(uint8_t patrn_idx)
{
	uint16_t npat;
	uint8_t i, idx, size, f = 0;
	uint8_t n_steps = 0;

	struct spr_pattern_set *ps = &spr_pattern[patrn_idx];

	if (ps->allocated)
		return true;

	for (i= 0; i < ps->n_states; i++) {
		n_steps += ps->state_steps[i];
	}
	ps->n_steps = n_steps;

	size = ps->size;
	if (ps->size == SPR_SIZE_32x16)
		size = 8;
	if (ps->size == SPR_SIZE_32x32)
		size = 16;

	npat = ps->n_planes * ps->n_steps * size;

	for (i = 0; i < vdp_hw_max_patterns - 1; i++) {
		f = f * spr_patt_valloc[i] + spr_patt_valloc[i];
		if (f == npat) {
			idx = i - npat + 1;
			sys_memset(&spr_patt_valloc[idx], 0, npat);
			vdp_memcpy(vdp_base_sppat_grp1 + idx * 8, ps->patterns, npat * 8);
			sys_memcpy(ps->colors2, ps->colors, ps->n_planes * ps->n_steps);
			ps->pidx = idx;
			ps->allocated = true;
			return true;
		}
	}
	return false;
}

void spr_vfree_pattern_set(uint8_t patrn_idx)
{
	uint8_t npat, size;

	struct spr_pattern_set *ps = &spr_pattern[patrn_idx];

	size = ps->size;
	if (ps->size == SPR_SIZE_32x16)
		size = 8;
	if (ps->size == SPR_SIZE_32x32)
		size = 16;

	npat = ps->n_planes * ps->n_steps * size;
	ps->allocated = false;
	sys_memset(&spr_patt_valloc[ps->pidx], 1, npat);
}

bool spr_is_allocated(uint8_t patrn_idx)
{
	struct spr_pattern_set *ps = &spr_pattern[patrn_idx];
	return ps->allocated;
}

static void spr_calc_patterns(struct spr_sprite_def *sp) __nonbanked
{
	uint8_t i, color_frame, base = 0, base2, frame;

	struct spr_pattern_set *ps = sp->pattern_set;
	for (i = 0; i < sp->cur_state; i++) {
		base += ps->state_steps[i];
	}
	color_frame = base + sp->cur_anim_step;

	switch (ps->size) {
		case SPR_SIZE_16x16:
			base *= (ps->size * ps->n_planes);
			frame = sp->cur_anim_step * (ps->size * ps->n_planes);
			for (i = 0; i < ps->n_planes; i++) {
				(sp->planes[i]).color = (ps->colors2)[color_frame];
				(sp->planes[i]).pattern = ps->pidx + base + frame + i * ps->size;
			}
			break;
		case SPR_SIZE_16x32:
			base *= (SPR_SIZE_16x16 * ps->n_planes);
			base2 = base + ps->n_planes * ps->n_steps * SPR_SIZE_16x16;
			frame = sp->cur_anim_step * (SPR_SIZE_16x16 * ps->n_planes);
			for (i = 0; i < ps->n_planes; i++) {
				// 2 is the max number of planes supported
				(sp->planes[i]).color = (ps->colors2)[color_frame];
				(sp->planes[i + 2]).color = (ps->colors2)[color_frame];
				(sp->planes[i]).pattern = ps->pidx + base + frame + i * SPR_SIZE_16x16;
				(sp->planes[i + 2]).pattern = ps->pidx + base2 + frame + i * SPR_SIZE_16x16;
			}
			break;
		case SPR_SIZE_32x16:
			base *= (SPR_SIZE_16x32 * ps->n_planes); // 0 8 16 32
			base2 = base + 4;
			frame = sp->cur_anim_step * SPR_SIZE_16x32 * ps->n_planes; // 0 or 8
			for (i = 0; i < ps->n_planes; i++) {
				// 2 is the max number of planes supported
				(sp->planes[i]).color = (ps->colors2)[color_frame];
				(sp->planes[i + 2]).color = (ps->colors2)[color_frame];
				(sp->planes[i]).pattern = ps->pidx + base + frame + i * SPR_SIZE_16x32;
				(sp->planes[i + 2]).pattern = ps->pidx + base2 + frame + i * SPR_SIZE_16x32;
			}
			break;
		case SPR_SIZE_32x32:
			// only 1 plane supported
			base *= SPR_SIZE_16x32;
			base2 = base + ps->n_steps * SPR_SIZE_16x32;
			frame = sp->cur_anim_step * SPR_SIZE_16x32;
			(sp->planes[0]).color = (ps->colors2)[color_frame];
			(sp->planes[1]).color = (ps->colors2)[color_frame];
			(sp->planes[2]).color = (ps->colors2)[color_frame];
			(sp->planes[3]).color = (ps->colors2)[color_frame];
			(sp->planes[0]).pattern = ps->pidx + base + frame;
			(sp->planes[1]).pattern = ps->pidx + base + frame + 4;
			(sp->planes[2]).pattern = ps->pidx + base2 + frame;
			(sp->planes[3]).pattern = ps->pidx + base2 + frame + 4;
			break;
	}
}

void spr_update(struct spr_sprite_def *sp) __nonbanked
{
	uint8_t i;
	spr_calc_patterns(sp);
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i], (uint8_t *)&sp->planes[i], 4);
		if (sp->pattern_set->size == SPR_SIZE_16x32
			|| sp->pattern_set->size == SPR_SIZE_32x16) {
			sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 1], (uint8_t *)&sp->planes[i + 2], 4);
		} else if (sp->pattern_set->size == SPR_SIZE_32x32) {
			sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 1], (uint8_t *)&sp->planes[1], 4);
			sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 2], (uint8_t *)&sp->planes[2], 4);
			sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 3], (uint8_t *)&sp->planes[3], 4);
		}
	}
}

/**
 * spr_show: finds a gap to allocate the attribute set
 */
uint8_t spr_show(struct spr_sprite_def *sp) __nonbanked
{
	uint8_t i, idx = 7, n, f = 0;
	n = sp->pattern_set->n_planes;
	if (sp->pattern_set->size == SPR_SIZE_16x32
		|| sp->pattern_set->size == SPR_SIZE_32x16)
		n = n * 2;
	else if (sp->pattern_set->size == SPR_SIZE_32x32)
		n = n * 4;
	for (i = 0; i < vdp_hw_max_sprites - 1; i++) {
		f = f * spr_attr_valloc[i] + spr_attr_valloc[i];
		if (f == n) {
			idx = i - n + 1;
			sys_memset(&spr_attr_valloc[idx], 0, n);
			sp->aidx = idx;
			spr_update(sp);
			return true;
		}
	}
	return false;
}

void spr_hide(struct spr_sprite_def *sp) __nonbanked
{
	uint8_t n, idx;
	struct vdp_hw_sprite null_spr;

	n = sp->pattern_set->n_planes;
	if (sp->pattern_set->size == SPR_SIZE_16x32
		|| sp->pattern_set->size == SPR_SIZE_32x16)
		n = n * 2;
	else if (sp->pattern_set->size == SPR_SIZE_32x32)
		n = n * 4;
	idx = sp->aidx;
	sys_memset(&spr_attr_valloc[idx], 1, n);

	/** set sprite outside screen using EC bit */
	null_spr.y = 193;
	null_spr.x = 0;
	null_spr.pattern = 0;
	null_spr.color = 128; // EC bit

	// FIXME: still wrong handling of multiple planes
	sys_memcpy((uint8_t *)&spr_attr[sp->aidx], (uint8_t *)&null_spr, sizeof(struct vdp_hw_sprite));
	if (sp->pattern_set->size == SPR_SIZE_16x32
		|| sp->pattern_set->size == SPR_SIZE_32x16) {
		sys_memcpy((uint8_t *)&spr_attr[sp->aidx + 1], (uint8_t *)&null_spr, sizeof(struct vdp_hw_sprite));
	} else if (sp->pattern_set->size == SPR_SIZE_32x32) {
		// TODO
	}
}

void spr_set_pos(struct spr_sprite_def *sp, uint8_t xp, uint8_t yp) __nonbanked
{
	uint8_t i;
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		(sp->planes[i]).x = xp;
		(sp->planes[i]).y = yp;
		if (sp->pattern_set->size == SPR_SIZE_16x32) {
			(sp->planes[i+ 2]).x = xp;
			(sp->planes[i+ 2]).y = yp + 16;
		} else if (sp->pattern_set->size == SPR_SIZE_32x16) {
			(sp->planes[i+ 2]).x = xp + 16;
			(sp->planes[i+ 2]).y = yp;
		} else if (sp->pattern_set->size == SPR_SIZE_32x32) {
			(sp->planes[1]).x = xp + 16;
			(sp->planes[1]).y = yp;
			(sp->planes[2]).x = xp;
			(sp->planes[2]).y = yp + 16;
			(sp->planes[3]).x = xp + 16;
			(sp->planes[3]).y = yp + 16;
		}
	}
}

void spr_set_plane_colors(struct spr_sprite_def *sp, uint8_t *colors) __nonbanked
{
	uint8_t i;
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		(sp->planes[i]).color = colors[i];
		if (sp->pattern_set->size == SPR_SIZE_16x32
			|| sp->pattern_set->size == SPR_SIZE_32x16) {
			(sp->planes[i + 2]).color = colors[i];
		} else if (sp->pattern_set->size == SPR_SIZE_32x32) {
			(sp->planes[1]).color = colors[i];
			(sp->planes[2]).color = colors[i];
			(sp->planes[3]).color = colors[i];
		}
	}
}

/**
 * Handle sprite animation for simple cases of 2 and 4 states with collision
 */
void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy) __nonbanked
{
	uint8_t old_dir, x, y;
	struct spr_pattern_set *ps = sp->pattern_set;

	old_dir = sp->cur_state;

	/* update state based on direction of movement */
	if (sp->pattern_set->n_states < 2) {
		// keep current state, no changes
	} else if (sp->pattern_set->n_states < 3) {

		if (dx > 0) {
			sp->cur_state = SPR_STATE_RIGHT;
		} else if (dx < 0) {
			sp->cur_state = SPR_STATE_LEFT;
		}

	} else if (sp->pattern_set->n_states < 5) {

		if (dx > 0) {
			sp->cur_state = SPR_STATE_RIGHT;
		} else if (dx < 0) {
			sp->cur_state = SPR_STATE_LEFT;
		}
		if (dy > 0) {
			sp->cur_state = SPR_STATE_DOWN;
		} else if (dy < 0) {
			sp->cur_state = SPR_STATE_UP;
		}

	} else {
		log_e("Only 2 or 4 states supported\n");
	}

	/* update animation frame */
	if (old_dir == sp->cur_state) {
		sp->anim_ctr++;
		//if (collision) {
			/* animate faster when colliding */
		//	sp->anim_ctr++;
		//}
		if (sp->anim_ctr > sp->anim_ctr_treshold) {
			sp->cur_anim_step++;
			sp->anim_ctr = 0;
		}
	} else {
		sp->cur_anim_step = 0;
	}

	if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
		sp->cur_anim_step = 0;

}
