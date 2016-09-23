/*
 * RetroDeLuxe Engine MSX1
 *
 * Copyright (C) 2013 Enric Geijo
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
byte spr_attr_valloc[vdp_hw_max_sprites];

// sprite pattern allocation table
byte spr_patt_valloc[vdp_hw_max_patterns];

/**
 * spr_init: initialize vdp sprites and allocation tables
 */
void spr_init(char spritesize, char zoom)
{
	vdp_init_hw_sprites(spritesize, zoom);
	// set all atributes out of screen
	vdp_memset(vdp_base_spatr_grp1, sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites, 212);
	sys_memset(spr_attr_valloc, 1, vdp_hw_max_sprites);
	sys_memset(spr_patt_valloc, 1, vdp_hw_max_patterns);
}

/**
 * spr_valloc_pattern_set:
 *		finds a gap to allocate a pattern set
 */
byte spr_valloc_pattern_set(struct spr_sprite_pattern_set *ps)
{
	uint npat;
	byte i, idx, f = 0;

	npat = ps->n_planes * ps->n_dirs * ps->n_anim_steps * ps->size;

	for (i = 0; i < vdp_hw_max_patterns - 1; i++) {
		f = f * spr_patt_valloc[i] + spr_patt_valloc[i];
		if (f == npat) {
			idx = i - npat + 1;
			sys_memset(&spr_patt_valloc[idx], 0, npat);
			vdp_copy_to_vram(ps->patterns, vdp_base_sppat_grp1 + idx * 8, npat * 8);
			ps->pidx = idx;
			return true;
		}
	}
	return false;
}

void spr_vfree_pattern_set(struct spr_sprite_pattern_set *ps)
{
	byte npat;
	npat = ps->n_planes * ps->n_dirs * ps->n_anim_steps * ps->size;
	sys_memset(&spr_patt_valloc[ps->pidx], 1, npat);	
}

static void spr_calc_patterns(struct spr_sprite_def *sp)
{
	byte i, base, frame;
	
	struct spr_sprite_pattern_set *ps = sp->pattern_set;

	base = sp->cur_dir * ps->n_anim_steps * (ps->size * ps->n_planes);
	frame = sp->cur_anim_step * (ps->size * ps->n_planes);
	for (i = 0; i < ps->n_planes; i++)
		(sp->planes[i]).pattern = ps->pidx + base + frame + i * ps->size ;
}

void spr_update(struct spr_sprite_def *sp)
{
	byte i;
	spr_calc_patterns(sp);
	for (i = 0; i < sp->pattern_set->n_planes; i++)
		vdp_set_hw_sprite(sp->aidx + i, &sp->planes[i]);
}

/**
 * spr_show: finds a gap to allocate the attribute set
 */
byte spr_show(struct spr_sprite_def *sp)
{
	byte i, idx, n, f = 0;
	n = sp->pattern_set->n_planes;
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

void spr_set_pos(struct spr_sprite_def *sp, byte xp, byte yp)
{
	byte i;
	for (i = 0; i < sp->pattern_set->n_planes; i++) {
		(sp->planes[i]).x = xp;
		(sp->planes[i]).y = yp;
	}
}

void spr_set_plane_colors(struct spr_sprite_def *sp, byte * colors)
{
	byte i;
	for (i = 0; i < sp->pattern_set->n_planes; i++)
		(sp->planes[i]).color = colors[i];
}

/**
 * How does it work when there are only 2 directions....
 *
 */
void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy, char collision)
{
	byte old_dir, x, y;

	old_dir = sp->cur_dir;

	if (sp->pattern_set->n_dirs < 3) {
		// handle 2 directions
		if (dx > 0)
			sp->cur_dir = 1;
		if (dx < 0)
			sp->cur_dir = 0;
	} else if (sp->pattern_set->n_dirs < 5) {
		// handle 4 directions
		if (dx > 0)
			sp->cur_dir = 3;
		if (dx < 0)
			sp->cur_dir = 1;
		if (dy > 0)
			sp->cur_dir = 0;
		if (dy < 0)
			sp->cur_dir = 2;
	} else {
		// handle 8 directions
		//sp->cur_dir = dir - 1;
	}

	if (old_dir == sp->cur_dir) {

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

	if (sp->cur_anim_step > sp->pattern_set->n_anim_steps - 1)
		sp->cur_anim_step = 0;

	x = (sp->planes[0]).x + dx;
	y = (sp->planes[0]).y + dy;
	if (!collision) {
		spr_set_pos(sp, x, y);
	}
	spr_update(sp);
}
