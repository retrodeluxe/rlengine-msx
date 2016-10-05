/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "gen/game_test.h"
#include <stdlib.h>

struct tile_set logo;
struct tile_set kv;
struct map_object_item *item;

struct spr_sprite_pattern_set monk_patt;
struct spr_sprite_pattern_set templar_patt;
struct spr_sprite_def monk;
struct spr_sprite_def templar_spr;

struct work_struct physics;
struct work_struct animate;

byte x,y,d;

byte fb[768];

void monk_physics_work();
void monk_animate_work();
byte tile_on_screen_coords(byte x, byte y);

struct monk_data {
	byte jump;
	byte jump_cnt;
	byte left;
	byte right;
	byte xpos;
	byte ypos;
	byte collision;
} monk_state;

#define COLLISION_UP_MASK 3
#define COLLISION_LEFT_MASK 12
#define COLLISION_RIGHT_MASK 48
#define COLLISION_DOWN_MASK 192

void main()
{
	byte i,d;
	byte *ptr;
	byte *src;
	byte tile;
	int map_xx, map_yy;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	spr_init(1,0);
	sys_irq_init();
	wq_start();

	INIT_TILE_SET(kv, tiles2);
	// need an offset of one
	tile_set_to_vram(&kv, 1);

	// show room 10
	map_xx =0; map_yy = 11;
	map_inflate_screen(map_cmpr_dict, map, fb, map_w, map_xx, map_yy);
	vdp_fastcopy_nametable(fb);

	SPR_DEFINE_PATTERN_SET(monk_patt, SPR_SIZE_16x32, 1, 2, 3, monk1);
	SPR_DEFINE_PATTERN_SET(templar_patt, SPR_SIZE_16x32, 1, 2, 2, templar);

	SPR_DEFINE_SPRITE(monk, &monk_patt, 10, monk1_color);
	SPR_DEFINE_SPRITE(templar_spr, &templar_patt, 10, templar_color);

	spr_valloc_pattern_set(&monk_patt);
	spr_valloc_pattern_set(&templar_patt);

	x = 100; y = 100;
	spr_set_pos(&monk, x, y);
	spr_show(&monk);
	spr_set_pos(&templar_spr, 10, 100);
	spr_show(&templar_spr);

	INIT_WORK(physics, monk_physics_work);
	INIT_WORK(animate, monk_animate_work);

	monk_state.jump  = 0;
	monk_state.jump_cnt = 0;

	do {
		d = sys_get_stick(0);
		if (d == 1 || d == 2 || d == 8 ) {
			if (!monk_state.jump) {
				tile = tile_on_screen_coords(x, y + 32);
				if (monk_state.collision & COLLISION_DOWN_MASK) {
					monk_state.jump = 1;
				}
			}
		}
		if (d == 3) {
			monk_state.right = 1;
		}	
		if (d == 7) {
			monk_state.left = 1; 
		}
		if (!monk_state.jump && !physics.pending)
				queue_delayed_work(&physics,0, 100);

		if (!animate.pending)
			queue_work(&animate);

		if (x > 240) {
			x = 0;
			spr_set_pos(&monk, x, y);
			vdp_screen_disable();
			map_xx += 32; map_yy = 11;
			map_inflate_screen(map_cmpr_dict, map, fb, map_w, map_xx, map_yy);
			vdp_fastcopy_nametable(fb);
			vdp_screen_enable();
		}

		// debug collisions
		/*vdp_fastcopy_nametable(fb);
		vdp_poke(vdp_base_names_grp1 + (x + 5)/ 8 + (y + 7)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x + 10)/ 8 + (y + 7)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x - 1)/ 8 + (y + 10)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x +18)/ 8 + (y + 10)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x - 1)/ 8 + (y + 28)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x +18)/ 8 + (y + 28)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x + 5)/ 8 + (y + 34)/ 8 * 32, 20);
		vdp_poke(vdp_base_names_grp1 + (x + 10)/8 + (y + 34)/ 8 * 32, 20);*/

	} while (1);
}

byte tile_on_screen_coords(byte x, byte y)
{
	byte *buff;
	buff = fb + x / 8 + (y / 8) * 32;
	return *buff;
}


int is_colliding_tile(byte tile)
{
	if (tile == 0 || tile > 100)
		return 0;
	return 1;
}

// check collisions properly
// 
void monk_check_collision()
{
	byte tile[8];
	byte i;
	//     0  1 
	//    2    4
	//    3    5
	//     6  7    
	tile[0] = tile_on_screen_coords(x + 5,  y + 7);
	tile[1] = tile_on_screen_coords(x + 10, y + 7);
	tile[2] = tile_on_screen_coords(x - 1,  y + 10);
	tile[3] = tile_on_screen_coords(x - 1,  y + 28);
	tile[4] = tile_on_screen_coords(x + 15, y + 10);
	tile[5] = tile_on_screen_coords(x + 15, y + 28);
	tile[6] = tile_on_screen_coords(x + 5,  y + 34);
	tile[7] = tile_on_screen_coords(x + 10, y + 34);

	// add two additional tiles between 23 and 45 to increase

	monk_state.collision = 0;
	for (i=0; i<8; i++) {
		if (is_colliding_tile(tile[i]))
			monk_state.collision |= 1 << i;
	}
}

void monk_physics_work()
{
	monk_check_collision();
	if(!(monk_state.collision & COLLISION_DOWN_MASK)) {
		spr_animate(&monk,0,2,0);
		y+=2;
	}
}

void monk_animate_work()
{
	signed char dx,dy;

	monk_check_collision();
	dx =0; dy =0;
	if (monk_state.jump) {
	 	if (!(monk_state.collision & COLLISION_UP_MASK))
			dy = - 2;
		if(++monk_state.jump_cnt > 25) {
			monk_state.jump = 0;
			monk_state.jump_cnt = 0;
		}
	}
	if (monk_state.left && !(monk_state.collision & COLLISION_LEFT_MASK)) {
		dx = - 1;
		monk_state.left = 0;
	}
	else if (monk_state.right && !(monk_state.collision & COLLISION_RIGHT_MASK)) {
		dx = 1;
		monk_state.right = 0;
	}

	if (dx != 0 || dy != 0) {
		spr_animate(&monk,dx,dy,0);
		x = x + dx;
		y = y + dy; 
	}
}


