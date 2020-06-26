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

#define DISP_OBJECT_SPRITE 1
#define DISP_OBJECT_TILE 2

struct displ_object;

struct animator {
	struct list_head list;
	void (*run)(struct displ_object *obj);
};

struct displ_object {
	uint8_t type; 	/*< sprite or dynamic tile */

	/* static animation data */
	uint8_t max;	/*<  max coordinate */
	uint8_t min;	/*<  min coordinate */
	uint8_t speed;	/*<  speed */
	uint8_t color;
	bool visible;

	/* collision detection flags */
	bool check_collision;

	/* dynamic animation data */
	uint8_t state;
	uint8_t aux;		// per object auxiliary data
	uint8_t aux2;
	int16_t xpos;
	int16_t ypos;
	uint8_t collision_state;
	struct spr_sprite_def *spr;
	struct tile_object *tob;
	struct list_head list;
	struct list_head animator_list;
};

void dpo_simple_animate(struct displ_object *dpo, signed char dx, signed char dy) __nonbanked;

#endif
