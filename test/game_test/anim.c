#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "dpo.h"
#include "phys.h"
#include "list.h"
#include "sfx.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"

#pragma CODE_PAGE 6

struct animator animators[MAX_ANIMATORS];

extern struct displ_object dpo_jean;

extern void add_bullet(uint8_t xpos, uint8_t ypos, uint8_t patrn_id,
			uint8_t anim_id, uint8_t state, uint8_t dir,
			uint8_t speed, struct displ_object *parent);

extern void anim_gargolyne(struct displ_object *obj);
extern void anim_death(struct displ_object *obj);
extern void anim_scythe(struct displ_object *obj);
extern void anim_satan(struct displ_object *obj);
extern void anim_satan_bullets(struct displ_object *obj);
extern void anim_dragon_flame(struct displ_object *obj);
extern void anim_dragon_bullets(struct displ_object *obj);
extern void anim_hanging_priest(struct displ_object *obj);
extern void anim_intro_chase(struct displ_object *obj);
extern void anim_intro_jean(struct displ_object *obj);
extern void anim_explosion(struct displ_object *obj);
extern void anim_red_parchment(struct displ_object *obj);
extern void anim_evil_chamber(struct displ_object *obj);
extern void anim_jean_bonfire(struct displ_object *obj);
extern void anim_block_crosses(struct displ_object *obj);
extern void anim_cross(struct displ_object *obj);
extern void anim_templar_bonfire(struct displ_object *obj);
extern void anim_close_door_satan(struct displ_object *obj);

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

void change_room() __nonbanked
{
	game_state.change_room = false;

	if (dpo_jean.xpos > 247) {
		game_state.jean_x = -8;
		game_state.jean_y = dpo_jean.ypos;
		game_state.room += 1;
		game_state.change_room = true;

	} else if (dpo_jean.xpos < -8) {
		game_state.jean_x = 247;
		game_state.jean_y = dpo_jean.ypos;
		game_state.room -= 1;
		game_state.change_room = true;
	}

	if (dpo_jean.ypos > 144) {
		game_state.jean_y = -24;
		dpo_jean.ypos = -24;
		game_state.jean_x = dpo_jean.xpos;
		game_state.room += 5;
		game_state.change_room = true;

	} else if (dpo_jean.ypos < -24 && game_state.room != ROOM_HAGMAN_TREE) {
		game_state.jean_y = 144;
		dpo_jean.ypos = 159;
		game_state.jean_x = dpo_jean.xpos;
		game_state.room -= 5;
		game_state.change_room = true;
	}

	if (game_state.change_room) {
		//log_e("room change state %d\n", dpo_jean.state);
		game_state.jean_state = dpo_jean.state;
		game_state.jean_collision_state = dpo_jean.collision_state;
		game_state.jean_anim_state = dpo_jean.spr->cur_state;

		/** final boss room is a checkpoint **/
		if (game_state.room == ROOM_SATAN) {
			game_state.checkpoint_x = 21;
			game_state.checkpoint_y = game_state.jean_y;
			game_state.checkpoint_room = game_state.room;
		}
	}
}

void anim_jean_death(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	sp->cur_state = JANE_STATE_DEATH;
	sp->anim_ctr++;
	if (sp->anim_ctr > sp->anim_ctr_treshold) {
		sp->cur_anim_step++;
		sp->anim_ctr = 0;
	}
	if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
		sp->cur_anim_step = 0;

	spr_update(sp);
}

void anim_jean(struct displ_object *obj)
{
	static uint8_t death_ct = 0;
	int8_t dx, dy;
	static int16_t jmp = 0;
	uint8_t x, y, fallthrough;

	#define CROUCH_OFFSET 8

	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	dx = 0;
	dy = 0;
	fallthrough = 0;

	x = (sp->planes[0]).x;
	y = (sp->planes[0]).y;

	//log_e("state in %d\n", obj->state);

	switch(obj->state) {
		case STATE_COLLISION:
			obj->state = STATE_DEATH;
			sfx_play_effect(SFX_DEATH, 0);
			return;
		case STATE_DEATH:
			death_ct++;
			if (death_ct < 20) {
				anim_jean_death(obj);
				return;
			}
			death_ct = 0;
			game_state.death = true;
			return;
		case STATE_JUMPING:
			dy = jmp++;
			if (jmp == 0) {
				obj->state = STATE_FALLING;
			}
			if (stick == STICK_LEFT) {
				sp->cur_state = JANE_STATE_LEFT_JUMP;
				dx = -3;
			} else if (stick == STICK_RIGHT) {
				sp->cur_state = JANE_STATE_RIGHT_JUMP;
				dx = 3;
			}
			if (is_colliding_up(obj)) {
				obj->state = STATE_FALLING;
				jmp = 0;
			}
			break;
		case STATE_MOVING_LEFT:
			sp->cur_state = JANE_STATE_LEFT;
			if (stick == STICK_LEFT) {
				dx = -2;
			} else if (stick == STICK_RIGHT) {
				obj->state = STATE_MOVING_RIGHT;
			} else if (stick == STICK_DOWN
				|| stick == STICK_DOWN_LEFT) {
				obj->state = STATE_CROUCHING;
				sp->cur_state = JANE_STATE_LEFT_CROUCH;
			} else if (stick == STICK_CENTER) {
				obj->state = STATE_IDLE;
			}
			if (trigger) {
				sp->cur_state = JANE_STATE_LEFT_JUMP;
				obj->state = STATE_JUMPING;
				jmp = -8;
				sfx_play_effect(SFX_JUMP, 0);
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_MOVING_RIGHT:
			sp->cur_state = JANE_STATE_RIGHT;
			if (stick == STICK_RIGHT) {
				dx = 2;
			} else if (stick == STICK_LEFT) {
				obj->state = STATE_MOVING_LEFT;
			} else if (stick == STICK_DOWN
				|| stick == STICK_DOWN_RIGHT) {
				obj->state = STATE_CROUCHING;
				sp->cur_state = JANE_STATE_RIGHT_CROUCH;
			} else if (stick == STICK_CENTER) {
				sp->cur_state = JANE_STATE_RIGHT;
				obj->state = STATE_IDLE;
			} else if (stick == STICK_CENTER) {
				obj->state = STATE_IDLE;
			}
			if (trigger) {
				sp->cur_state = JANE_STATE_RIGHT_JUMP;
				obj->state = STATE_JUMPING;
				jmp = -8;
				sfx_play_effect(SFX_JUMP, 0);
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_CROUCHING:
			if (stick == STICK_DOWN ) {
				if (is_colliding_down_ft(obj) && !is_colliding_down(obj)) {
					dy = 8;
					fallthrough = 1;
				}
			} else if (stick == STICK_DOWN_LEFT) {
				dx = -2;
				sp->cur_state = JANE_STATE_LEFT_CROUCH;
			} else if (stick == STICK_DOWN_RIGHT) {
				dx = 2;
				sp->cur_state = JANE_STATE_RIGHT_CROUCH;
			} else if (stick == STICK_CENTER && !is_colliding_up(obj)) {
				if (sp->cur_state == JANE_STATE_LEFT_CROUCH)
					sp->cur_state = JANE_STATE_LEFT;
				else if (sp->cur_state == JANE_STATE_RIGHT_CROUCH)
					sp->cur_state = JANE_STATE_RIGHT;
				obj->state = STATE_IDLE;
			} else if (stick == STICK_LEFT && !is_colliding_up(obj)) {
				obj->state = STATE_MOVING_LEFT;
				sp->cur_state = JANE_STATE_LEFT;
			} else if (stick == STICK_LEFT && is_colliding_up(obj)) {
				dx = -2;
				sp->cur_state = JANE_STATE_LEFT_CROUCH;
			} else if (stick == STICK_RIGHT && !is_colliding_up(obj)) {
				obj->state = STATE_MOVING_RIGHT;
				sp->cur_state = JANE_STATE_RIGHT;
			} else if (stick == STICK_RIGHT && is_colliding_up(obj)) {
				dx = 2;
				sp->cur_state = JANE_STATE_RIGHT_CROUCH;
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_IDLE:
			jmp = 0;
			if (stick == STICK_LEFT) {
				obj->state = STATE_MOVING_LEFT;
			} else if (stick == STICK_RIGHT) {
				obj->state = STATE_MOVING_RIGHT;
			} else if (stick == STICK_DOWN
				|| stick == STICK_DOWN_LEFT
				|| stick == STICK_DOWN_RIGHT) {
				obj->state = STATE_CROUCHING;
				if (sp->cur_state == JANE_STATE_LEFT)
					sp->cur_state = JANE_STATE_LEFT_CROUCH;
				else if (sp->cur_state == JANE_STATE_RIGHT)
					sp->cur_state = JANE_STATE_RIGHT_CROUCH;
			}
			if (trigger) {
				obj->state = STATE_JUMPING;
				jmp = -8;
				sfx_play_effect(SFX_JUMP, 0);
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_FALLING:
			dy = jmp;
			if (jmp++ > 6) jmp = 6;
			if (sp->cur_state == JANE_STATE_RIGHT
				|| sp->cur_state == JANE_STATE_RIGHT_CROUCH) {
					sp->cur_state = JANE_STATE_RIGHT_JUMP;
			} else if (sp->cur_state == JANE_STATE_LEFT
				|| sp->cur_state == JANE_STATE_LEFT_CROUCH) {
					sp->cur_state = JANE_STATE_LEFT_JUMP;
			}
			if (stick == STICK_LEFT) {
				sp->cur_state = JANE_STATE_LEFT_JUMP;
				dx = -2;
			} else if (stick == STICK_RIGHT) {
				sp->cur_state = JANE_STATE_RIGHT_JUMP;
				dx = 2;
			}
			if (is_colliding_down(obj) || is_colliding_down_ft(obj)) {
				if (sp->cur_state == JANE_STATE_RIGHT_JUMP) {
					sp->cur_state = JANE_STATE_RIGHT;
				} else if (sp->cur_state == JANE_STATE_LEFT_JUMP) {
					sp->cur_state = JANE_STATE_LEFT;
				}
				obj->state = STATE_IDLE;
			}
			break;
	}

	/** handle collisions and update sprite **/
	if (obj->state == STATE_CROUCHING) {
		phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, true, true);
	} else {
		phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false, true);
	}

	if (obj->state != STATE_IDLE) {
		sp->anim_ctr++;
		if (sp->anim_ctr > sp->anim_ctr_treshold) {
			sp->cur_anim_step++;
			sp->anim_ctr = 0;
		}
		if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
			sp->cur_anim_step = 0;
	}

	if ((dx > 0 && !is_colliding_right(obj))
		|| (dx < 0 && !is_colliding_left(obj))) {
    		obj->xpos += dx;
	}
	if ((dy > 0 && !is_colliding_down(obj) && !is_colliding_down_ft(obj))
		|| (dy < 0 && !is_colliding_up(obj))) {
		obj->ypos += dy;
	}

	if (fallthrough && is_colliding_down_ft(obj)) {
		obj->ypos += dy;
		fallthrough = 0;
	}

	if (obj->state == STATE_CROUCHING) {
		/* crouching requires temporary offset to jean sprite */
		spr_set_pos(sp, obj->xpos, obj->ypos + CROUCH_OFFSET);
		spr_update(sp);
		/* restore pos */
		spr_set_pos(sp, obj->xpos, obj->ypos);
	} else {
		//log_e("set pos x %d, y %d\n", obj->xpos, obj->ypos);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}


	//log_e("state %d pos %d %d col %d dx %d dy %d jmp %d\n", obj->state, obj->xpos, obj->ypos, obj->collision_state, dx ,dy, jmp);
}

void anim_cycle_tile(struct displ_object *dpo)
{
	if (dpo->state++ == 5) {
		if (dpo->tob->cur_anim_step < dpo->tob->ts->n_frames) {
			tile_object_show(dpo->tob, scr_tile_buffer, true);
			dpo->tob->cur_anim_step++;
		} else {
			dpo->tob->cur_anim_step = 0;
		}
		dpo->state = 0;
	}
        if (dpo->state == 100) {
                tile_object_hide(dpo->tob, scr_tile_buffer, true);
        }
}

/**
 * tile cycling for lava
 */
void anim_lava(struct displ_object *dpo)
{
	if (dpo->state++ == 10) {
		if (dpo->tob->cur_anim_step < dpo->tob->ts->n_frames) {
			tile_object_show(dpo->tob, scr_tile_buffer, true);
			dpo->tob->cur_anim_step++;
		} else {
			dpo->tob->cur_anim_step = 0;
		}
		dpo->state = 0;
	}
}

/**
 * Simple up-down animation bounded by max/min coordinates
 */
void anim_up_down_bounded(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dy = 0;

	dy = obj->speed;
	switch(obj->state) {
		case STATE_MOVING_DOWN:
			if (obj->ypos > obj->max) {
				obj->state = STATE_MOVING_UP;
				dy *= -1;;
			}
			break;
		case STATE_MOVING_UP:
			dy *= -1;
			if (obj->ypos < obj->min) {
				obj->state = STATE_MOVING_DOWN;
				dy *= -1;
			}
			break;
		default:
			obj->state = STATE_MOVING_DOWN;
			break;
	}

	obj->ypos += dy;
	spr_animate(sp, 0, dy);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}


/**
 * Simple left-right animation bounded by max/min coordinates
 */
void anim_left_right_bounded(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx = 0;

	dx = obj->speed;
	switch(obj->state) {
		case STATE_MOVING_LEFT:
			if (obj->xpos > obj->max) {
				obj->state = STATE_MOVING_RIGHT;
				dx *= -1;;
			}
			break;
		case STATE_MOVING_RIGHT:
			dx *= -1;
			if (obj->xpos < obj->min) {
				obj->state = STATE_MOVING_LEFT;
				dx *= -1;
			}
			break;
		default:
			obj->state = STATE_MOVING_LEFT;
			break;
	}

	obj->xpos += dx;
	spr_animate(sp, dx, 0);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

/**
 * Animation for a Templar chasing Jean in the forest
 */
void anim_chase_forest(struct displ_object *obj)
{
	int8_t dx = 0, dy = 0;
	struct spr_sprite_def *sp = obj->spr;

	dx = 2;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 2;
			if (obj->xpos > 204 && obj->xpos < 210) {
				obj->state = STATE_FALLING_RIGHT;
			} else if (obj->xpos > 154 && obj->xpos < 170) {
				obj->state = STATE_HOPPING_RIGHT;
				obj->aux = 0;
			} else if (obj->xpos == 140) {
				obj->state = STATE_HOPPING_RIGHT;
				obj->aux = 0;
			}
			break;
		case STATE_HOPPING_RIGHT:
			obj->aux++;
			if (obj->aux < 5) {
				dy = -2;
				dx = 2;
			} else if (obj->aux < 15) {
				dy = 0;
			}
			if (obj->ypos < 112) {
				obj->ypos = 112;
				obj->state = STATE_MOVING_RIGHT;
			} else if (obj->ypos == 120) {
				obj->ypos = 120;
				obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_FALLING_RIGHT:
			dy = 4;
			dx = 2;
			if (obj->ypos > 124) {
				obj->ypos = 124;
			 	obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_OFF_SCREEN:
			if (obj->xpos > -32) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			break;
	}

	obj->xpos += dx;
	obj->ypos += dy;
	if (obj->visible) {
		spr_animate(sp, dx, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * Animation for a Templar chasing Jean in the forest
 */
void anim_chase_graveyard(struct displ_object *obj)
{
	int8_t dx = 0, dy = 0;
	struct spr_sprite_def *sp = obj->spr;

	dx = 2;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 2;
			if (obj->xpos == 72
				|| obj->xpos == 118
				|| obj->xpos == 150) {
				obj->state = STATE_HOPPING_RIGHT;
				obj->aux = 0;
			} else if (obj->xpos > 200 && obj->xpos < 230) {
				obj->state = 8;
				obj->aux = 0;
			} else if (obj->xpos > 247) {
				dx = 0;
			}
			break;
		case STATE_HOPPING_RIGHT:
			obj->aux++;
			if (obj->aux < 4) {
				dy = -2;
				dx = 2;
			} else if (obj->aux < 8) {
				dy = 0;
			} else {
				obj->state = STATE_FALLING_RIGHT;
			}
			break;
		case 8:
			obj->aux++;
			if (obj->aux < 5) {
				dy = -2;
				dx = 2;
			} else if (obj->aux < 7) {
				dy = 0;
			} else {
				obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_FALLING_RIGHT:
			dy = 4;
			dx = 2;
			if (obj->ypos > 124) {
				obj->ypos = 124;
			 	obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_OFF_SCREEN:
			if (obj->xpos > -32) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			break;
	}

	obj->xpos += dx;
	obj->ypos += dy;
	if (obj->visible) {
		spr_animate(sp, dx, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * Animation to show the door closing after jeans enters the church, then the
 * templars keep banging on it
 */
void anim_close_door(struct displ_object *obj)
{
	if (game_state.door_trigger) {
		if (!obj->visible) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->visible = true;
			obj->state = 0;
			obj->tob->cur_dir = 1;
			obj->tob->cur_anim_step = 0;
		}
		if (obj->state == 20 || obj->state == 21) {
			obj->tob->cur_dir = 0;
			if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
				obj->tob->cur_anim_step++;
				if (obj->tob->cur_anim_step > 1)
					obj->tob->cur_anim_step = 0;
				tile_object_show(obj->tob, scr_tile_buffer, true);
				sfx_play_effect(SFX_DOOR, 0);
			}
		} else if (obj->state == 22) {
			obj->tob->cur_dir = 0;
			obj->tob->cur_anim_step = 0;
			obj->state = 0;
		}
		obj->state++;
	}
}

void anim_open_door(struct displ_object *obj)
{
	if (game_state.toggle[2] != 0) {
		tile_object_hide(obj->tob, scr_tile_buffer, true);
	}
}

/**
 * Animation for bullets thrown up and to one side,
 *      following an inverted parabolic trajectory
 */
void anim_falling_bullets(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx = 0, dy = 0;

	if (obj->aux2 == 1) {
		dx = 2;
	} else if (obj->aux2 == 0) {
		dx = -2;
	}

	// inverted parabola with slope derivative = -1
	dy = obj->aux++;

	obj->xpos += dx;
	obj->ypos += dy;

	if (obj->ypos > 160 || obj->xpos > 250) {
		spr_hide(obj->spr);
		list_del(&obj->list);
		obj->state = 255;
	} else {
		spr_animate(sp, dx, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * Animation of plants throwing bullets
 */
void anim_plant(struct displ_object *obj)
{
	// obj->aux contains delay in cycles, state rolls over at 200
	if (obj->state == obj->aux) {
		// open
		obj->tob->cur_anim_step = 1;
		tile_object_show(obj->tob, scr_tile_buffer, true);
		add_bullet(obj->xpos,
			obj->ypos - 8,
			PATRN_SMALL_BULLET, ANIM_FALLING_BULLETS, 0, -4, 0, NULL);
		sfx_play_effect(SFX_SHOOT,0);
	} else if (obj->state == obj->aux + 5) {
		add_bullet(obj->xpos + 8,
			obj->ypos - 8,
			PATRN_SMALL_BULLET, ANIM_FALLING_BULLETS, 0, -4, 1, NULL);
	} else if (obj->state == obj->aux + 10) {
		// close
		obj->tob->cur_anim_step = 0;
		tile_object_show(obj->tob, scr_tile_buffer, true);
	}
	if (++obj->state > 70) obj->state = 0;
}

/**
 * Animation of a water dropping from the ceiling
 */
void anim_waterdrop(struct displ_object *obj)
{
	uint8_t max = obj->max;
	// here we assume all drops start at the same position
	if (obj->state == 0) {
		obj->aux2 = obj->ypos;
		obj->aux = 0;
		obj->state = 1;
	}
	// state 1 - just count
	if (obj->state == 1) {
		obj->spr->cur_anim_step = 0;
		if (obj->aux++ > 30) {
			obj->aux = 0;
			obj->state = 2;
		}
	}
	if (obj->state == 2) {
		obj->spr->cur_anim_step = 1;
		if (obj->aux++ > 30) {
			obj->aux = 0;
			obj->state = 3;
		}
	}
	if (obj->state == 3 && obj->ypos < max) {
		obj->ypos += 4;
		obj->spr->cur_anim_step = 2;
		spr_set_pos(obj->spr, obj->xpos, obj->ypos);
	}
	if (obj->ypos >= max) {
		obj->aux = 0;
		obj->state = 0;
		obj->ypos = obj->aux2;
		obj->spr->cur_anim_step = 0;
		spr_set_pos(obj->spr,obj->xpos, obj->ypos);
	}
	spr_update(obj->spr);
}

#define SPLASH_LAUNCH 0
#define SPLASH_WAIT 1
#define SPLASH_RECEIVE 2

/**
 * Animation of a fish jumping from the water
 */
void anim_fish_jump(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx = 0, dy = 0;

	// inverted parabola with slope derivative = -0.33
	dy = obj->aux;
	obj->ypos += dy;
	if (obj->aux2 == 2) {
		obj->aux++;
		obj->aux2 = 0;
	} else {
		obj->aux2++;
	}

	if (obj->ypos > 155) {
		spr_hide(obj->spr);
		list_del(&obj->list);
		obj->state = 255;
		obj->parent->aux2 = SPLASH_RECEIVE;
	} else {
		spr_animate(sp, 0, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * 3-frame animation of a splash of water produced by a fish
 */
void anim_splash(struct displ_object *obj)
{
	if (obj->aux2 == SPLASH_LAUNCH) {
		obj->state++;
		if (obj->state == obj->aux) {
			obj->tob->cur_anim_step = 0;
			tile_object_show(obj->tob, scr_tile_buffer, true);
			add_bullet(obj->xpos,
				obj->ypos - 8,
				PATRN_FISH, ANIM_FISH_JUMP, 0, -6, 0, obj);
		} else if (obj->state == obj->aux + 2) {
			obj->tob->cur_anim_step = 1;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else if (obj->state == obj->aux + 5) {
			obj->tob->cur_anim_step = 2;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else if (obj->state > obj->aux + 10) {
			obj->state = 0;
			obj->aux2 = SPLASH_WAIT;
			tile_object_hide(obj->tob, scr_tile_buffer, true);
		}
	} else if (obj->aux2 == SPLASH_RECEIVE) {
		obj->state++;
		if (obj->state == 0) {
			obj->tob->cur_anim_step = 0;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else if (obj->state == 2) {
			obj->tob->cur_anim_step = 1;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else if (obj->state == 5) {
			obj->tob->cur_anim_step = 2;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else if (obj->state > 10) {
			obj->state = 0;
			obj->aux2 = SPLASH_LAUNCH;
			tile_object_hide(obj->tob, scr_tile_buffer, true);
		}
	}
}

/**
 * The Ghost moves slowly towards jean, taking into account collisions
 */
void anim_ghost(struct displ_object *obj)
{
	static int8_t dx, dy;
	struct spr_sprite_def *sp = obj->spr;

	if (obj->aux == 10) {
		dx = obj->speed;
		dy = obj->speed;
		if (obj->xpos > dpo_jean.xpos) {
			dx *= -1;
		}
		if (obj->ypos > dpo_jean.ypos) {
			dy *= -1;
		}
		if (is_colliding(obj)) {
			if (is_colliding_x(obj)
				&& sys_rand() > 128) {
				dy *= -1;
			}
			if (is_colliding_y(obj)
				&& sys_rand() > 128) {
				dx *= -1;
			}
		}
	}
	if (obj->aux++ > 10) obj->aux = 0;

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false, false);

	if ((dx > 0 && !is_colliding_right(obj))
		|| (dx < 0 && !is_colliding_left(obj))) {
		obj->xpos += dx;
	}
	if ((dy > 0 && !is_colliding_down(obj))
		|| (dy < 0 && !is_colliding_up(obj))) {
		obj->ypos += dy;
	}

	spr_animate(sp, dx, dy);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

/**
 * Archer skeleton throws arrows and switches direction based on jean's position
 */
void anim_archer_skeleton(struct displ_object *obj)
{
	uint8_t arrow_xpos;

	if (dpo_jean.xpos > (obj->xpos + 16)) {
		obj->tob->cur_dir = 1;
		arrow_xpos = obj->xpos + 16;
	} else if (dpo_jean.xpos < (obj->xpos - 16)) {
		obj->tob->cur_dir = 0;
		arrow_xpos = obj->xpos - 4;
	}

	if (obj->state == obj->aux) {
		obj->tob->cur_anim_step = 0;
		tile_object_show(obj->tob, scr_tile_buffer, true);
		add_bullet(obj->xpos,
			obj->ypos + 4,
			PATRN_ARROW,
			ANIM_HORIZONTAL_PROJECTILE,
			obj->tob->cur_dir, obj->tob->cur_dir, 8, NULL);
		sfx_play_effect(SFX_SHOOT,0);
	} else if (obj->state == obj->aux + 10) {
		obj->tob->cur_anim_step = 1;
		tile_object_show(obj->tob, scr_tile_buffer, true);
	} else if (obj->state == 0) {
		tile_object_show(obj->tob, scr_tile_buffer, true);
	}
	if (++obj->state > 60) obj->state = 1;
}

/**
 * animation for arrows and other horizontal projectiles
 */
void anim_horizontal_projectile(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx;

	if (obj->aux == 0) dx = -obj->aux2;
	else dx = obj->aux2;

	obj->xpos += dx;
	if (obj->xpos < 4 || obj->xpos > 235) {
		spr_hide(obj->spr);
		list_del(&obj->list);
		obj->state = 255;
	} else {
		spr_animate(sp, dx, 0);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * Animation of wall mounted gargolyne that spits a fireball
 */
void anim_gargolyne(struct displ_object *obj)
{
	uint8_t spit_xpos;

	if (obj->state == obj->aux) {
		obj->tob->cur_anim_step = 1;
		tile_object_show(obj->tob, scr_tile_buffer, true);
		add_bullet(obj->xpos,
			obj->ypos + 4,
			PATRN_SPIT,
			ANIM_HORIZONTAL_PROJECTILE, 0, 0, 8, NULL);
		sfx_play_effect(SFX_SHOOT, 0);
	} else if (obj->state == obj->aux + 10) {
		obj->tob->cur_anim_step = 0;
		tile_object_show(obj->tob, scr_tile_buffer, true);
	} else if (obj->state == 0) {
		tile_object_show(obj->tob, scr_tile_buffer, true);
	}

	if (++obj->state > 60) obj->state = 1;
}

/**
 * Water animation by tileobject shift
 */
void anim_water(struct displ_object *obj)
{
	if (dpo->state++ == 10) {
		dpo->tob->idx++;
		if (dpo->tob->idx > 7) dpo->tob->idx = 0;
		tile_object_show(dpo->tob, scr_tile_buffer, true);
		dpo->state = 0;
	}
}

void init_animators()
{
	uint8_t i;

	for (i=0; i< MAX_ANIMATORS; i++) {
		animators[i].page = 6;
	}

	animators[ANIM_JEAN].run = anim_jean;
	animators[ANIM_CYCLE_TILE].run = anim_cycle_tile;
	animators[ANIM_CHASE_FOREST].run = anim_chase_forest;
	animators[ANIM_CHASE_GRAVEYARD].run = anim_chase_graveyard;
	animators[ANIM_CLOSE_DOOR].run = anim_close_door;
	animators[ANIM_SHOOTER_PLANT].run = anim_plant;
	animators[ANIM_FALLING_BULLETS].run = anim_falling_bullets;
	animators[ANIM_WATERDROP].run = anim_waterdrop;
	animators[ANIM_FISH_JUMP].run = anim_fish_jump;
	animators[ANIM_SPLASH].run = anim_splash;
	animators[ANIM_GHOST].run = anim_ghost;
	animators[ANIM_UP_DOWN_BOUNDED].run = anim_up_down_bounded;
	animators[ANIM_ARCHER_SKELETON].run = anim_archer_skeleton;
	animators[ANIM_HORIZONTAL_PROJECTILE].run = anim_horizontal_projectile;
	animators[ANIM_GARGOLYNE].run = anim_gargolyne;
	animators[ANIM_LEFT_RIGHT_BOUNDED].run = anim_left_right_bounded;
	animators[ANIM_LAVA].run = anim_lava;
	animators[ANIM_WATER].run = anim_water;
	animators[ANIM_OPEN_DOOR].run = anim_open_door;

	animators[ANIM_INTRO_CHASE].page = 7;
	animators[ANIM_INTRO_CHASE].run = anim_intro_chase;
	animators[ANIM_INTRO_JEAN].page = 7;
	animators[ANIM_INTRO_JEAN].run = anim_intro_jean;
	animators[ANIM_DEATH].page = 7;
	animators[ANIM_DEATH].run = anim_death;
	animators[ANIM_SCYTHE].page = 7;
	animators[ANIM_SCYTHE].run = anim_scythe;
	animators[ANIM_SATAN].page = 7;
	animators[ANIM_SATAN].run = anim_satan;
	animators[ANIM_SATAN_BULLETS].page = 7;
	animators[ANIM_SATAN_BULLETS].run = anim_satan_bullets;
	animators[ANIM_DRAGON_FLAME].page = 7;
	animators[ANIM_DRAGON_FLAME].run = anim_dragon_flame;
	animators[ANIM_DRAGON_BULLETS].page = 7;
	animators[ANIM_DRAGON_BULLETS].run = anim_dragon_bullets;
	animators[ANIM_HANGING_PRIEST].page = 7;
	animators[ANIM_HANGING_PRIEST].run = anim_hanging_priest;
	animators[ANIM_EXPLOSION].page = 7;
	animators[ANIM_EXPLOSION].run = anim_explosion;
	animators[ANIM_RED_PARCHMENT].page = 7;
	animators[ANIM_RED_PARCHMENT].run = anim_red_parchment;
	animators[ANIM_EVIL_CHAMBER].page = 7;
	animators[ANIM_EVIL_CHAMBER].run = anim_evil_chamber;
	animators[ANIM_JEAN_BONFIRE].page = 7;
	animators[ANIM_JEAN_BONFIRE].run = anim_jean_bonfire;
	animators[ANIM_BLOCK_CROSSES].page = 7;
	animators[ANIM_BLOCK_CROSSES].run = anim_block_crosses;
	animators[ANIM_CROSS].page = 7;
	animators[ANIM_CROSS].run = anim_cross;
	animators[ANIM_TEMPLAR_BONFIRE].page = 7;
	animators[ANIM_TEMPLAR_BONFIRE].run = anim_templar_bonfire;
	animators[ANIM_CLOSE_DOOR_SATAN].page = 7;
	animators[ANIM_CLOSE_DOOR_SATAN].run = anim_close_door_satan;
}
