/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2019 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#include "dpo.h"
#include "phys.h"
#include "sprite.h"

 /**
  * Handle sprite animation for simple cases of 2 and 4 states with collision
  */
 void dpo_simple_animate(struct displ_object *dpo, signed char dx, signed char dy)
 {
 	uint8_t old_dir, x, y;
	struct spr_sprite_def *sp = dpo->spr;
 	struct spr_pattern_set *ps = sp->pattern_set;

	x = (sp->planes[0]).x;
	y = (sp->planes[0]).y;
	
	if (dpo-> type == DISP_OBJECT_SPRITE) {
		spr_animate(sp, dx, dy);

		if (!is_colliding_x(dpo)) {
    			dpo->xpos += dx;
			x += dx;
		}

		if (!is_colliding_y(dpo)) {
			dpo->ypos += dy;
			y += dy;
		}

 		spr_set_pos(sp, x, y);
 		spr_update(sp);
	}
 }
