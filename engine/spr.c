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
#include "vdp.h"
#include "sprite.h"
#include "log.h"

static byte spr_attr_valloc[vdp_hw_max_sprites];
static byte spr_patt_valloc[vdp_hw_max_patterns];

const struct spr_delta_pos spr_stick2coords[] = {
	{0, -2}, {1, -1}, {2, 0}, {1, 1},
	{0, 2}, {-1, 1}, {-2, 0}, {-1, -1}
};

void spr_init(char spritesize, char zoom)
{
	vdp_init_hw_sprites(spritesize, zoom);
	// FIXME : I think empty patterns should be off screen.
	vdp_memset(vdp_base_spatr_grp1,
		   sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites, 0);
	sys_memset(spr_attr_valloc, 1, vdp_hw_max_sprites);
	sys_memset(spr_patt_valloc, 1, vdp_hw_max_patterns);
}

static byte array__find_gap(byte * array, byte size)
{
	byte i, nfree;

	nfree = 0;
	for (i = 0; i < vdp_hw_max_patterns - 1; i++) {
		nfree = nfree * array[i] + array[i];
		if (nfree == size)
			return i - size + 1;
	}
	return 1;
}

byte spr_valloc(struct spr_sprite_def * sp)
{
	byte ipat, iatt;
	uint npat;

	npat = sp->n_planes * sp->n_dirs * sp->n_anim_steps * sp->size;
	ipat = array__find_gap(spr_patt_valloc, npat);
	iatt = array__find_gap(spr_attr_valloc, sp->n_planes);
	if (ipat == 1 || iatt == 1)
		return 1;
	sys_memset(&spr_attr_valloc[iatt], 0, sp->n_planes);
	sys_memset(&spr_patt_valloc[ipat], 0, npat);
	sp->base_hw_patt = ipat;
	sp->base_hw_attr = iatt;
	vdp_copy_to_vram(sp->patterns, vdp_base_sppat_grp1 + ipat * 8,
			 npat * 8);
	return 0;
}

void spr_vfree(struct spr_sprite_def *sp)
{
	byte npat;

	npat = sp->n_planes * sp->n_dirs * sp->n_anim_steps * sp->size;
	sys_memset(&spr_attr_valloc[sp->base_hw_attr], 1, sp->n_planes);
	sys_memset(&spr_patt_valloc[sp->base_hw_patt], 1, npat);
}

static void spr_calc_patterns(struct spr_sprite_def *sp)
{
	byte i, ppd, b1, b2;

	ppd = sp->n_planes * sp->n_anim_steps;
	b1 = sp->cur_dir * ppd;
	b2 = sp->cur_anim_step * sp->n_planes;
	for (i = 0; i < sp->n_planes; i++)
		(sp->planes[i]).pattern = (b1 + b2 + i) * sp->size;
}

void spr_show(struct spr_sprite_def *sp)
{
	byte i;

	spr_calc_patterns(sp);
	for (i = 0; i < sp->n_planes; i++)
		vdp_set_hw_sprite_di((byte *) & (sp->planes[i]),
				     sp->base_hw_attr + i);

}

void spr_hide(struct spr_sprite_def *sp)
{
	vdp_memset(vdp_base_spatr_grp1 +
		   sp->base_hw_attr * sizeof(struct vdp_hw_sprite),
		   sizeof(struct vdp_hw_sprite) * sp->n_planes, 0);
}

void spr_set_pos(struct spr_sprite_def *sp, byte xp, byte yp)
{
	byte i;
	for (i = 0; i < sp->n_planes; i++) {
		(sp->planes[i]).x = xp;
		(sp->planes[i]).y = yp;
	}
}

void spr_set_plane_colors(struct spr_sprite_def *sp, byte * colors)
{
	byte i;
	for (i = 0; i < sp->n_planes; i++)
		(sp->planes[i]).color = colors[i];
}

void spr_move(struct spr_sprite_def *sp, byte dir, byte steps, char collision)
{
	byte old_dir, x, y;
	char dx, dy;

	old_dir = sp->cur_dir;

	dx = spr_stick2coords[dir - 1].dx * steps;
	dy = spr_stick2coords[dir - 1].dy * steps;

	if (sp->n_dirs < 5) {
		/* hack because bad ordering of sprite data... FIXME */
		if (dx > 0)
			sp->cur_dir = 1;
		if (dx < 0)
			sp->cur_dir = 2;
		if (dy > 0)
			sp->cur_dir = 0;
		if (dy < 0)
			sp->cur_dir = 3;
	} else {
		sp->cur_dir = dir - 1;
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

	if (sp->cur_anim_step > sp->n_anim_steps - 1)
		sp->cur_anim_step = 0;

	x = (sp->planes[0]).x + dx;
	y = (sp->planes[0]).y + dy;
	if (!collision)
		spr_set_pos(sp, x, y);
	spr_show(sp);
}

static void spr_handler_5th_sprite_interleaving()
{
	// no idea how to do this...
}
