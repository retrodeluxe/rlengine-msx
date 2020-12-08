/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "map.h"
#include "tile.h"
#include "gen/sheeep.h"
#include "log.h"
#include "phys.h"

enum spr_patterns_t {
   PATRN_SHEEPY,
   PATRN_INDUSTRIAL,
};

enum spr_states {
	RIGHT_IDLE,
	RIGHT_SQUASH,
	LEFT_SQUASH,
	LEFT_IDLE,
	RIGHT_RUN,
	LEFT_RUN,
	RIGHT_ACTION,
	LEFT_ACTION,
};

SpriteDef sheep;
SpriteDef indust;
TileSet tilest_map;
DisplayObject dpo_main;

// credits for robert norenberg (art)

uint8_t screen_buffer[768];

void my_spr_animate(SpriteDef *sp, signed char dx, signed char dy, char collision);

void main()
{
	uint8_t x = 100,y = 40;
	uint8_t i, stick, trigg, jump = 0, squash = 0;
	uint8_t jump_ct = 0, squash_ct = 0;
	int8_t dx, dy, dx_8, dy_8, dx_jump;
	uint16_t j;

	uint8_t state_steps[] = {3, 5, 5, 3, 8, 8, 4, 4};

	vdp_set_mode(MODE_GRP2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear(0);
	spr_init();
	sys_irq_init();

	INIT_TILE_SET(tilest_map, tiles);
	tile_set_to_vram(&tilest_map, 1);
	sys_memset(screen_buffer, 0, 768);
	sys_memcpy(screen_buffer, map_tilemap, 768 - 64);


	phys_init();
	for (i = 1; i < 127; i++ ) {
		phys_set_colliding_tile(i);
	}

	//phys_clear_colliding_tile(33);
	phys_clear_colliding_tile(65);
	phys_clear_colliding_tile(25);
	phys_clear_colliding_tile(26);
	phys_clear_colliding_tile(27);
	phys_clear_colliding_tile(28);
	phys_clear_colliding_tile(29);
	phys_clear_colliding_tile(30);
	phys_clear_colliding_tile(31);
	phys_clear_colliding_tile(32);
	phys_clear_colliding_tile(60);
	phys_clear_colliding_tile(61);
	phys_clear_colliding_tile(62);
	phys_clear_colliding_tile(63);
	phys_clear_colliding_tile(64);
	phys_clear_colliding_tile(65);
	phys_clear_colliding_tile(57);
	phys_clear_colliding_tile(58);
	phys_clear_colliding_tile(84);
	phys_clear_colliding_tile(85);
	phys_clear_colliding_tile(86);
	phys_clear_colliding_tile(87);
	phys_clear_colliding_tile(89);
	phys_clear_colliding_tile(91);
	phys_clear_colliding_tile(123);
	phys_clear_colliding_tile(118);
	phys_clear_colliding_tile(119);
	phys_clear_colliding_tile(124);
	phys_clear_colliding_tile(125);
	phys_clear_colliding_tile(135);
	phys_clear_colliding_tile(92);
	phys_clear_colliding_tile(93);
	phys_clear_colliding_tile(94);
	phys_clear_colliding_tile(95);
	phys_clear_colliding_tile(96);
	phys_clear_colliding_tile(97);
	phys_clear_colliding_tile(53);
	phys_clear_colliding_tile(54);
	phys_clear_colliding_tile(102);
	phys_clear_colliding_tile(103);
	phys_clear_colliding_tile(71);
	phys_clear_colliding_tile(126);

	phys_clear_colliding_tile(90);
	phys_clear_colliding_tile(70);
	phys_clear_colliding_tile(102);
	//phys_clear_colliding_tile(110);
	phys_clear_colliding_tile(122);
	phys_clear_colliding_tile(121);
	phys_clear_colliding_tile(59);



	SPR_DEFINE_PATTERN_SET(PATRN_INDUSTRIAL, SPR_SIZE_16x16, 1, 8, state_steps, industrial);

	spr_init_sprite(&indust,PATRN_INDUSTRIAL);

	if(!spr_valloc_pattern_set(PATRN_INDUSTRIAL)) {
		log_e("FATAL: could not allocate sprite\n");
	}

	dpo_main.xpos = x;
	dpo_main.ypos = y;
	dpo_main.type = DISP_OBJECT_SPRITE;
	dpo_main.state = 0;
	dpo_main.spr = &indust;
	dpo_main.collision_state = 0;

	spr_set_pos(&indust, x,y);
	spr_show(&indust);
	indust.cur_state = RIGHT_IDLE;
	indust.cur_anim_step = 0;
	do {
			dx =0; dy =0; dy_8 = 0; dx_8 = 0;
			stick = sys_get_stick(0);
			trigg = sys_get_trigger(0);
			sys_memcpy(screen_buffer, map_tilemap, 768 - 64);


			if (trigg && !squash && !jump && indust.cur_state != RIGHT_ACTION && indust.cur_state != LEFT_ACTION) {
				indust.cur_anim_step = 0;
				if (indust.cur_state == RIGHT_IDLE || indust.cur_state == RIGHT_RUN)
					indust.cur_state = RIGHT_ACTION;
				else if (indust.cur_state == LEFT_IDLE || indust.cur_state == LEFT_RUN)
					indust.cur_state = LEFT_ACTION;
			} else if (!jump && !squash && indust.cur_state != RIGHT_ACTION && indust.cur_state != LEFT_ACTION){
				if (stick == 3) {
					indust.cur_state = RIGHT_RUN;
					dx = 2;
					if(++indust.state_anim_ctr[RIGHT_RUN] > 5 ) {
						indust.state_anim_ctr[RIGHT_RUN] = 0;
						if (++indust.cur_anim_step >= indust.pattern_set->state_steps[RIGHT_RUN]) {
							indust.cur_anim_step = 0;
						}
					}
				} else if (stick == 7) {
					indust.cur_state = LEFT_RUN;
					dx = -2;
					if(++indust.state_anim_ctr[LEFT_RUN] > 5 ) {
						indust.state_anim_ctr[LEFT_RUN] = 0;
						if (++indust.cur_anim_step >= indust.pattern_set->state_steps[LEFT_RUN]) {
							indust.cur_anim_step = 0;
						}
					}
				} else if (stick == 1 || stick == 2 || stick == 8) {
					jump = 1;
					if (stick == 2) {
						indust.cur_state = RIGHT_RUN;
					}
					if (stick == 8) {
						indust.cur_state = LEFT_RUN;
					}
					if (indust.cur_state == RIGHT_IDLE || indust.cur_state == RIGHT_RUN) {
						indust.cur_anim_step = 0;
						indust.cur_state = RIGHT_RUN;
						if (indust.cur_state == RIGHT_RUN) {
							dx_jump = 2;
						}
					} else if (indust.cur_state == LEFT_IDLE || indust.cur_state == LEFT_RUN) {
						indust.cur_anim_step = 7;
						indust.cur_state = LEFT_RUN;
						if (indust.cur_state == LEFT_RUN) {
							dx_jump = -2;
						}
					}
					if (stick == 1) {
						dx_jump = 0;
					}
				} else {
					if (indust.cur_state == LEFT_RUN) {
						indust.cur_state = LEFT_IDLE;
					} else if (indust.cur_state == RIGHT_RUN) {
						indust.cur_state = RIGHT_IDLE;
					}
				}
			} else if (jump) {
				if (stick == 1 || stick == 2 || stick == 8) {
					if (jump_ct < 10) {
						dy_8 = -8;
					}
				}
				if (stick == 3 || stick == 2) {
					indust.cur_state = RIGHT_RUN;
					dx_8 = 8;
				} else if (stick == 7 || stick == 8) {
					dx_8 = -8;
					indust.cur_state = LEFT_RUN;
				} else {
					dx = 0;
					if (indust.cur_state == LEFT_RUN) {
						indust.cur_anim_step = 0;
						indust.cur_state = LEFT_IDLE;
					} else if (indust.cur_state == RIGHT_RUN) {
						indust.cur_anim_step = 0;
						indust.cur_state = RIGHT_IDLE;
					}
				}
			} else if (squash) {
				if (stick == 3) {
					dx = 1;
				} else if (stick == 7) {
					dx = -1;
				}
			}

			if (is_colliding_down(&dpo_main) && jump && jump_ct > 19) {
				squash = 1;
				jump = 0;
				jump_ct = 0;
				dx_jump = 0;
				if (indust.cur_state == RIGHT_RUN || indust.cur_state == RIGHT_IDLE) {
					indust.cur_state = RIGHT_SQUASH;
					indust.cur_anim_step = 0;
					dy = -2;
				} else if (indust.cur_state == LEFT_RUN || indust.cur_state == LEFT_IDLE) {
					indust.cur_state = LEFT_SQUASH;
					indust.cur_anim_step = 0;
					dy = -2;
				}
			}

			if (squash) {
				if (indust.cur_state == RIGHT_SQUASH) {
					if(++indust.state_anim_ctr[RIGHT_SQUASH] > 1 ) {
						indust.state_anim_ctr[RIGHT_SQUASH] = 0;
						if (++indust.cur_anim_step >= indust.pattern_set->state_steps[RIGHT_SQUASH]) {
							indust.cur_state = RIGHT_IDLE;
							indust.cur_anim_step = 0;
							squash = 0;
						}
					}
				} else if (indust.cur_state == LEFT_SQUASH) {
					if(++indust.state_anim_ctr[LEFT_SQUASH] > 1 ) {
						indust.state_anim_ctr[LEFT_SQUASH] = 0;
						if (++indust.cur_anim_step >= indust.pattern_set->state_steps[LEFT_SQUASH]) {
							indust.cur_state = LEFT_IDLE;
							indust.cur_anim_step = 0;
							squash = 0;
						}
					}
				}
			}

			if (jump) {
				jump_ct++;
				if (indust.cur_state == RIGHT_RUN || indust.cur_state == RIGHT_IDLE) {
					if (indust.cur_state == RIGHT_RUN) {
						dx = dx_jump + dx_8 / 8;
					} else if (indust.cur_state == RIGHT_IDLE) {
						dx = dx_jump;
					}
					if (jump_ct < 15) {
						dy = -1;
						dy += dy_8 /2;
					} else if (jump_ct < 20) {
						dy = 0;
						indust.cur_anim_step = 1;
					} else {
						dy = 2;
						indust.cur_anim_step = 2;
					}
				} else if (indust.cur_state == LEFT_RUN || indust.cur_state == LEFT_IDLE) {
					if (indust.cur_state == LEFT_RUN) {
						dx = dx_jump + dx_8 / 8;
					} else if (indust.cur_state == LEFT_IDLE) {
						dx = dx_jump;
					}
					if (jump_ct < 15) {
						dy = -1;
						dy += dy_8 /2;
					} else if (jump_ct < 20) {
						dy = 0;
						if (indust.cur_state == LEFT_IDLE) {
							indust.cur_anim_step = 1;
						} else {
							indust.cur_anim_step = 6;
						}
					} else {
						dy = 2;
						if (indust.cur_state == LEFT_IDLE) {
							indust.cur_anim_step = 2;
						} else {
							indust.cur_anim_step = 5;
						}
					}
				}
			}

			if (indust.cur_state == LEFT_ACTION) {
				if(++indust.state_anim_ctr[LEFT_ACTION] > 5 ) {
					indust.state_anim_ctr[LEFT_ACTION] = 0;
					if (++indust.cur_anim_step >= indust.pattern_set->state_steps[LEFT_ACTION]) {
						indust.cur_state = LEFT_IDLE;
						indust.cur_anim_step = 0;
					}
				}
			}

			if (indust.cur_state == RIGHT_ACTION) {
				if(++indust.state_anim_ctr[RIGHT_ACTION] > 5 ) {
					indust.state_anim_ctr[RIGHT_ACTION] = 0;
					if (++indust.cur_anim_step >= indust.pattern_set->state_steps[RIGHT_ACTION]) {
						indust.cur_state = RIGHT_IDLE;
						indust.cur_anim_step = 0;
					}
				}
			}


			if (indust.cur_state == RIGHT_IDLE) {
				if(++indust.state_anim_ctr[RIGHT_IDLE] > 5 ) {
					indust.state_anim_ctr[RIGHT_IDLE] = 0;
					if (++indust.cur_anim_step >= indust.pattern_set->state_steps[RIGHT_IDLE]) {
						indust.cur_anim_step = 0;
					}
				}
			}

			if (indust.cur_state == LEFT_IDLE) {
				if(++indust.state_anim_ctr[LEFT_IDLE] > 5 ) {
					indust.state_anim_ctr[LEFT_IDLE] = 0;
					if (++indust.cur_anim_step >= indust.pattern_set->state_steps[LEFT_IDLE]) {
						indust.cur_anim_step = 0;
					}
				}
			}

			if (!jump)
				dy = 2;

			phys_detect_tile_collisions(&dpo_main, screen_buffer, dx, dy, false);
			vdp_fastcopy_nametable(screen_buffer);
			if (!jump && !is_colliding_down(&dpo_main)) {
				if (indust.cur_state == LEFT_RUN) {
					indust.cur_anim_step = 5;
				} else {
					indust.cur_anim_step = 2;
				}
			}
			if (is_colliding_down(&dpo_main) && dy > 0) {
			//	log_e("collision down");
				dy = 0;
			}
			if (is_colliding_up(&dpo_main) && dy < 0)  {
				dy = 0;
			//	log_e("collision up");
			}
			if (is_colliding_left(&dpo_main) || is_colliding_right(&dpo_main)) {
				dx = 0;
			}

			x = (indust.planes[0]).x + dx;
			y = (indust.planes[0]).y + dy;
			//if (!collision) {
				spr_set_pos(&indust, x, y);
				dpo_main.xpos = x;
				dpo_main.ypos = y;
		//	}
			spr_update(&indust);



			//log_e("collision : %d\n", dpo_main.collision_state & COLLISION_DOWN);

			sys_sleep(10);
	} while (true);


	sys_reboot();
}
