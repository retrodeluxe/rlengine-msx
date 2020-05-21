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

#ifndef _MSX_H_PHYS
#define _MSX_H_PHYS

#include "dpo.h"

#define COLLISION_LEFT 	1
#define COLLISION_RIGHT	2
#define COLLISION_UP		4
#define COLLISION_DOWN		8
#define COLLISION_UP_LEFT	16
#define COLLISION_UP_RIGHT	32
#define COLLISION_DOWN_LEFT	64
#define COLLISION_DOWN_RIGHT	128

#define is_colliding_left(x)	(((x)->collision_state & COLLISION_LEFT) != 0)
#define is_colliding_right(x)	(((x)->collision_state & COLLISION_RIGHT) != 0)
#define is_colliding_down(x)	(((x)->collision_state & COLLISION_DOWN) != 0)
#define is_colliding_up(x)	(((x)->collision_state & COLLISION_UP) != 0)
#define is_colliding_x(x)	(is_colliding_left((x)) || is_colliding_right((x)))
#define is_colliding_y(x)	(is_colliding_up((x)) || is_colliding_down((x)))
#define is_colliding(x)		(((x)->collision_state) != 0)

#define MAX_CROUPS 12

struct tile_collision_group {
        uint8_t start;
        uint8_t end;
        uint8_t data;
        struct displ_object *dpo;
        void (*handler)(struct displ_object *dpo, uint8_t data);
};

enum tile_collision_type {
	TILE_COLLISION_FULL,
	TILE_COLLISION_DOWN,
	TILE_COLLISION_TRIGGER,
};

void phys_init();
void phys_set_sprite_collision_handler(void (*handler));
void phys_clear_sprite_collision_handler();
void phys_set_tile_collision_handler(struct displ_object *dpo, void (*handler), uint8_t data);
void phys_set_colliding_tile_object(struct displ_object *dpo, enum tile_collision_type type, void (*handler), uint8_t data);
void phys_clear_colliding_tile_object(struct displ_object *dpo);
void phys_set_colliding_tile(uint8_t tile);
void phys_set_down_colliding_tile(uint8_t tile);
void phys_set_trigger_colliding_tile(uint8_t tile);
void phys_clear_colliding_tile(uint8_t tile);
void phys_detect_tile_collisions(struct displ_object *obj, uint8_t *map, int8_t dx, int8_t dy);
void phys_detect_fall(struct displ_object *obj, uint8_t *map, int8_t dx);

#endif
