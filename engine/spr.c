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

// sprite attribute allocation table
uint8_t spr_attr_valloc[vdp_hw_max_sprites];

// sprite pattern allocation table
uint8_t spr_patt_valloc[vdp_hw_max_patterns];

// spr pattern sets
struct spr_pattern_set spr_pattern[SPR_PATRN_MAX];

/**
 * spr_init: initialize vdp sprites and allocation tables
 */
void spr_init(void)
{
	vdp_init_hw_sprites(SPR_SHOW_16x16, SPR_ZOOM_OFF);
	// set all atributes out of screen
	vdp_memset(vdp_base_spatr_grp1, sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites, 212);
	sys_memset(spr_attr_valloc, 1, vdp_hw_max_sprites);
	sys_memset(spr_patt_valloc, 1, vdp_hw_max_patterns);
	sys_memset(spr_pattern, 0,  sizeof(struct spr_pattern_set) * SPR_PATRN_MAX);
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
	uint8_t i, idx, f = 0;
	uint8_t n_steps = 0;

	struct spr_pattern_set *ps = &spr_pattern[patrn_idx];

	if (ps->allocated)
		return true;

	for (i= 0; i < ps->n_states; i++) {
		n_steps += ps->state_steps[i];
	}
	ps->n_steps = n_steps;

	npat = ps->n_planes * ps->n_steps * ps->size;

	for (i = 0; i < vdp_hw_max_patterns - 1; i++) {
		f = f * spr_patt_valloc[i] + spr_patt_valloc[i];
		if (f == npat) {
			idx = i - npat + 1;
			sys_memset(&spr_patt_valloc[idx], 0, npat);
			vdp_copy_to_vram(ps->patterns, vdp_base_sppat_grp1 + idx * 8, npat * 8);
			ps->pidx = idx;
			ps->allocated = true;
			return true;
		}
	}
	return false;
}

void spr_vfree_pattern_set(uint8_t patrn_idx)
{
	uint8_t npat;

	struct spr_pattern_set *ps = &spr_pattern[patrn_idx];

	npat = ps->n_planes * ps->n_steps * ps->size;
	ps->allocated = false;
	sys_memset(&spr_patt_valloc[ps->pidx], 1, npat);
}

static void spr_calc_patterns(struct spr_sprite_def *sp)
{
	uint8_t i, base = 0, base2, frame;

	struct spr_pattern_set *ps = sp->pattern_set;
	for (i = 0; i < sp->cur_state; i++) {
		base += ps->state_steps[i];
	}

	switch (ps->size) {
		case SPR_SIZE_16x16:
			base *= (ps->size * ps->n_planes);
			frame = sp->cur_anim_step * (ps->size * ps->n_planes);
			for (i = 0; i < ps->n_planes; i++) {
				(sp->planes[i]).pattern = ps->pidx + base + frame + i * ps->size;
			}
			break;
		case SPR_SIZE_16x32:
			base *= (SPR_SIZE_16x16 * ps->n_planes);
			base2 = base + ps->n_planes * ps->n_steps * SPR_SIZE_16x16;
			frame = sp->cur_anim_step * (SPR_SIZE_16x16 * ps->n_planes);
			for (i = 0; i < ps->n_planes; i++) {
				(sp->planes[i]).pattern = ps->pidx + base + frame + i * SPR_SIZE_16x16;
				(sp->planes[i+3]).pattern = ps->pidx + base2 + frame + i * SPR_SIZE_16x16;
			}
			break;
	}
}

void spr_update(struct spr_sprite_def *sp)
{
	uint8_t i;
	spr_calc_patterns(sp);
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		vdp_set_hw_sprite(&sp->planes[i], sp->aidx + i);
		if (sp->pattern_set->size == SPR_SIZE_16x32)
			vdp_set_hw_sprite(&sp->planes[i+3], sp->aidx + i + sp->pattern_set->n_planes);
	}

}

/**
 * spr_show: finds a gap to allocate the attribute set
 */
uint8_t spr_show(struct spr_sprite_def *sp)
{
	uint8_t i, idx, n, f = 0;
	n = sp->pattern_set->n_planes;
	if (sp->pattern_set->size == SPR_SIZE_16x32)
		n = n * 2;
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

void spr_hide(struct spr_sprite_def *sp)
{
	vdp_memset(vdp_base_spatr_grp1 +
		   sp->aidx * sizeof(struct vdp_hw_sprite),
		   sizeof(struct vdp_hw_sprite) * sp->pattern_set->n_planes, 0);
}

void spr_set_pos(struct spr_sprite_def *sp, uint8_t xp, uint8_t yp)
{
	uint8_t i;
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		(sp->planes[i]).x = xp;
		(sp->planes[i]).y = yp;
		if (sp->pattern_set->size == SPR_SIZE_16x32) {
			(sp->planes[i+ 3]).x = xp;
			(sp->planes[i+ 3]).y = yp + 16;
		}
	}
}

void spr_set_plane_colors(struct spr_sprite_def *sp, uint8_t *colors)
{
	uint8_t i;
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		(sp->planes[i]).color = colors[i];
		if (sp->pattern_set->size == SPR_SIZE_16x32) {
			(sp->planes[i + 3]).color = colors[i];
		}
	}
}

/**
 *
 */
void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy, char collision)
{
	uint8_t old_dir, x, y;
	struct spr_pattern_set *ps = sp->pattern_set;

	old_dir = sp->cur_state;

	if (sp->pattern_set->n_states < 3) {
		// handle 2 directions
		if (dx > 0)
			sp->cur_state = 1;
		if (dx < 0)
			sp->cur_state = 0;
	} else if (sp->pattern_set->n_states < 5) {
		// handle 4 directions
		if (dx > 0)
			sp->cur_state = 3;
		if (dx < 0)
			sp->cur_state = 1;
		if (dy > 0)
			sp->cur_state = 0;
		if (dy < 0)
			sp->cur_state = 2;
	} else {
		// handle 8 directions
		//sp->cur_state = dir - 1;
	}

	if (old_dir == sp->cur_state) {

		if (!collision)
			sp->anim_ctr++;
		else
			sp->anim_ctr += 2;

		if (sp->anim_ctr > sp->anim_ctr_treshold) {
			sp->cur_anim_step++;
			sp->anim_ctr = 0;
		}

	} else {
		sp->cur_anim_step = 0;
	}

	if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
		sp->cur_anim_step = 0;

	x = (sp->planes[0]).x + dx;
	y = (sp->planes[0]).y + dy;
	if (!collision) {
		spr_set_pos(sp, x, y);
	}
	spr_update(sp);
}
