/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */
 #include "msx.h"
 #include "log.h"
 #include "displ.h"
 #include "phys.h"
 #include "sys.h"


uint8_t colliding_tiles[32];
static uint8_t tile[12];


void phys_init()
{
	sys_memset(colliding_tiles, 0, 32);
}

/*
 * Set collision flag for a specific tile
 */
void phys_set_colliding_tile(uint8_t tile)
{
	uint8_t bit, byte;

	byte = tile >> 3;
	bit = tile & 7;
	colliding_tiles[byte] |= 1 << bit;
}

static bool is_coliding_tile_pair(uint8_t tile1, uint8_t tile2)
{
	uint8_t bit1, bit2, byte1, byte2;
	bool result;

	byte1 = tile1 >> 3;
	byte2 = tile2 >> 3;
	bit1 = tile1 & 7;
	bit2 = tile2 & 7;
	result = colliding_tiles[byte1] & (1 << bit1);
	result |= colliding_tiles[byte2] & (1 << bit2);
	return result;
}

static bool is_coliding_tile_triplet(uint8_t tile1, uint8_t tile2, uint8_t tile3)
{
	uint8_t bit1, bit2, bit3, byte1, byte2, byte3;
	bool result;

	byte1 = tile1 >> 3;
	byte2 = tile2 >> 3;
	byte3 = tile3 >> 3;
	bit1 = tile1 & 7;
	bit2 = tile2 & 7;
	bit3 = tile3 & 7;
	result = colliding_tiles[byte1] & (1 << bit1);
	result |= colliding_tiles[byte2] & (1 << bit2);
	result |= colliding_tiles[byte3] & (1 << bit3);
	return result;
}
 /*
  * Update collision state of a display object 16x16
  */
void phys_detect_tile_collisions(struct displ_object *obj, uint8_t *map)
{
	#define TILE_WIDTH 32
	uint8_t x,y;
	uint8_t *base;

	x = obj->xpos;
	y = obj->ypos;

	/* adjust box size  : this may have to be 4 */
	if (x >= 2) x = x - 2;
	if (y >= 2) y = y - 2;

	base = map + x / 8 + y / 8 * TILE_WIDTH;

	tile[0] = *(base);
	tile[1] = *(base + 1);
	tile[2] = *(base + 2);
	tile[3] = *(base + 3);
	tile[4] = *(base + TILE_WIDTH);
	tile[5] = *(base + TILE_WIDTH * 2);
	tile[6] = *(base + TILE_WIDTH * 3);
	tile[7] = *(base + TILE_WIDTH * 3 + 1);
	tile[8] = *(base + TILE_WIDTH * 3 + 2);
	tile[9] = *(base + TILE_WIDTH * 3 + 3);
	tile[10] = *(base + 3 + TILE_WIDTH);
	tile[11] = *(base + 3 + TILE_WIDTH * 2);

	obj->collision_state = 0;
	if (is_coliding_tile_pair(tile[4], tile[5])) {
		obj->collision_state |= COLLISION_LEFT;
	}
	if (is_coliding_tile_pair(tile[10], tile[11])) {
		obj->collision_state |= COLLISION_RIGHT;
	}
	if (is_coliding_tile_pair(tile[1], tile[2])) {
		obj->collision_state |= COLLISION_UP;
	}
	if (is_coliding_tile_pair(tile[7], tile[8])) {
		obj->collision_state |= COLLISION_DOWN;
	}
	if (is_coliding_tile_triplet(tile[4], tile[0], tile[1])) {
		obj->collision_state |= COLLISION_UP_LEFT;
	}
	if (is_coliding_tile_triplet(tile[2], tile[3], tile[10])) {
		obj->collision_state |= COLLISION_UP_RIGHT;
	}
	if (is_coliding_tile_triplet(tile[5], tile[6], tile[7])) {
		obj->collision_state |= COLLISION_DOWN_LEFT;
	}
	if (is_coliding_tile_triplet(tile[8], tile[9], tile[11])) {
		obj->collision_state |= COLLISION_DOWN_RIGHT;
	}
	//log_e("collision state %d\n", obj->collision_state);
}
