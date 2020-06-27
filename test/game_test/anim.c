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
extern void add_bullet(uint8_t xpos, uint8_t ypos, uint8_t patrn_id, uint8_t anim_id,
	uint8_t state, uint8_t dir, uint8_t speed, struct displ_object *parent);

extern void anim_gargolyne(struct displ_object *obj);

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

void change_room() __nonbanked
{
	game_state.change_room = false;

	if (dpo_jean.xpos > 239) {
		game_state.jean_x = 5;
		game_state.jean_y = dpo_jean.ypos;
		game_state.room += 1;
		game_state.change_room = true;

	} else if (dpo_jean.xpos < 0) {
		game_state.jean_x = 239;
		game_state.jean_y = dpo_jean.ypos;
		game_state.room -= 1;
		game_state.change_room = true;
	}

	if (dpo_jean.ypos > (192 - 50)) {
		game_state.jean_y = 1;
		game_state.jean_x = dpo_jean.xpos;
		game_state.room += 5;
		game_state.change_room = true;

	} else if (dpo_jean.ypos < 0) {
		game_state.jean_y = 192 - 64;
		game_state.jean_x = dpo_jean.xpos;
		game_state.room -= 5;
		game_state.change_room = true;
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
	static uint8_t jump_ct = 0, fall_ct = 0;
	static uint8_t death_ct = 0;
	static int8_t dy_8 = 0;
	int8_t dx, dy;
	uint8_t x, y, fallthrough;

	#define CROUCH_OFFSET 8

	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	dx = 0;
	dy = 0;
	fallthrough = 0;

	x = (sp->planes[0]).x;
	y = (sp->planes[0]).y;

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
			jump_ct++;

			if (jump_ct < 5) {
				dy = -4;
				dy += dy_8 / 2;
				dy_8 = 0;
			} else if (jump_ct < 10) {
				dy = 0;
			} else {
				jump_ct = 0;
				obj->state = STATE_FALLING;
			}
			if (stick == STICK_LEFT) {
				sp->cur_state = JANE_STATE_LEFT_JUMP;
				dx = -3;
			} else if (stick == STICK_RIGHT) {
				sp->cur_state = JANE_STATE_RIGHT_JUMP;
				dx = 3;
			}
			// handle trigger again for extended
			if (trigger) {
				if (jump_ct < 5) {
					dy_8 = -8;
				}
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
			} else if (stick == STICK_CENTER) {
				if (sp->cur_state == JANE_STATE_LEFT_CROUCH)
					sp->cur_state = JANE_STATE_LEFT;
				else if (sp->cur_state == JANE_STATE_RIGHT_CROUCH)
					sp->cur_state = JANE_STATE_RIGHT;
				obj->state = STATE_IDLE;
			} else if (stick == STICK_LEFT) {
				obj->state = STATE_MOVING_LEFT;
				sp->cur_state = JANE_STATE_LEFT;
			} else if (stick == STICK_RIGHT) {
				obj->state = STATE_MOVING_RIGHT;
				sp->cur_state = JANE_STATE_RIGHT;
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_IDLE:
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
				sfx_play_effect(SFX_JUMP, 0);
			}
			if (!is_colliding_down(obj) && !is_colliding_down_ft(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_FALLING:
			dy = 4;
			if (sp->cur_state == JANE_STATE_RIGHT
				|| sp->cur_state == JANE_STATE_RIGHT_CROUCH) {
					sp->cur_state = JANE_STATE_RIGHT_JUMP;
			} else if (sp->cur_state == JANE_STATE_LEFT
				|| sp->cur_state == JANE_STATE_LEFT_CROUCH) {
					sp->cur_state = JANE_STATE_LEFT_JUMP;
			}
			if (stick == STICK_LEFT) {
				sp->cur_state = JANE_STATE_LEFT_JUMP;
				dx = -3;
			} else if (stick == STICK_RIGHT) {
				sp->cur_state = JANE_STATE_RIGHT_JUMP;
				dx = 3;
			}
			if (is_colliding_down(obj) || is_colliding_down_ft(obj)) {
				if (sp->cur_state == JANE_STATE_RIGHT_JUMP) {
					sp->cur_state = JANE_STATE_RIGHT;
				} else if (sp->cur_state == JANE_STATE_LEFT_JUMP) {
					sp->cur_state = JANE_STATE_LEFT;
				}
				obj->state = STATE_IDLE;
				fall_ct = 0;
			}
			break;
	}

	/** handle collisions and update sprite **/
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, true);

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


	// log_e("state %d\n", obj->state);
	// log_e("dx %d dy %d\n", dx, dy);
	// log_e("x %d y %d\n", obj->xpos, obj->ypos);
	// log_e("collision %d\n", obj->collision_state);
}

void anim_cycle_tile(struct displ_object *dpo)
{
        // maybe I can just set the objet to gone.
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

void anim_left_right(struct displ_object *obj)
{
	int8_t dx = 0;
	switch(obj->state) {
		case STATE_MOVING_LEFT:
			dx = -2;
			if (is_colliding_left(obj)) {
				obj->state = STATE_MOVING_RIGHT;
				dx = 2;
			}
			break;
		case STATE_MOVING_RIGHT:
			dx = 2;
			if (is_colliding_right(obj)) {
				obj->state = STATE_MOVING_LEFT;
				dx = -2;
			}
			break;
		default:
			obj->state = STATE_MOVING_LEFT;
	}

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, 0, false);
	dpo_simple_animate(obj, dx, 0);
}

void anim_left_right_floor(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx = 0;
	switch(obj->state) {
		case STATE_MOVING_LEFT:
			dx = -2;
			if (is_colliding_left(obj)
				|| !is_colliding_down(obj)
				|| obj->xpos < 3) {
				obj->state = STATE_MOVING_RIGHT;
				dx = 2;
			}
			break;
		case STATE_MOVING_RIGHT:
			dx = 2;
			if (is_colliding_right(obj) || !is_colliding_down(obj)) {
				obj->state = STATE_MOVING_LEFT;
				dx = -2;
			}
			break;
		default:
			obj->state = STATE_MOVING_LEFT;
	}
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, 0, false);
	phys_detect_fall(obj, scr_tile_buffer, dx);

	if ((dx > 0 && !is_colliding_right(obj))
		|| (dx < 0 && !is_colliding_left(obj))) {
		obj->xpos += dx;
	}
	spr_animate(sp, dx, 0);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

void anim_up_down(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dy = 0;

	dy = obj->speed;
	switch(obj->state) {
		case STATE_MOVING_DOWN:
			if (is_colliding_down(obj)) {
				obj->state = STATE_MOVING_UP;
				dy *= -1;;
			}
			break;
		case STATE_MOVING_UP:
			dy *= -1;
			if (is_colliding_up(obj)) {
				obj->state = STATE_MOVING_DOWN;
				dy *= -1;
			}
			break;
	}

	phys_detect_tile_collisions(obj, scr_tile_buffer, 0, dy, false);

	if ((dy > 0 && !is_colliding_down(obj))
		|| (dy < 0 && !is_colliding_up(obj))) {
		obj->ypos += dy;
	}
	spr_animate(sp, 0, dy);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
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
	}

	obj->xpos += dx;
	spr_animate(sp, dx, 0);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

/**
 * Animation for a Templar chasing Jean in different screens
 */
void anim_chase(struct displ_object *obj)
{
	int8_t dx = 0, dy = 0;
	struct spr_sprite_def *sp = obj->spr;

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false);

	if (game_state.room == ROOM_GRAVEYARD)
		phys_detect_fall(obj, scr_tile_buffer, dx);

	obj->aux2++;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 2;
			if (is_colliding_right(obj)) {
				obj->state = STATE_HOPPING_RIGHT;
				obj->aux = 0;
			} else if (!is_colliding_right(obj) && !is_colliding_down(obj)) {
				if (game_state.room == ROOM_FOREST)
					obj->state = STATE_FALLING_RIGHT;
				else
					obj->state = STATE_HOPPING_RIGHT;
			}
			break;
		case STATE_HOPPING_RIGHT:
			obj->aux++;
			if (obj->aux < 5) {
				dy = -4;
				dx = 2;
			} else if (obj->aux < 10) {
				dy = 0;
			} else {
				dy = 4;
				dx = 2;
			}
			if (obj->aux > 10 && is_colliding_down(obj)) {
				obj->aux = 0;
				obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_FALLING_RIGHT:
			dy = 4;
			dx = 2;
			if (is_colliding_down(obj)) {
				obj->state = STATE_MOVING_RIGHT;
			}
			break;
		case STATE_OFF_SCREEN:
			if (obj->aux2 > 10) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			return;
		case STATE_OFF_SCREEN_DELAY_1S:
			if (obj->aux2 > 19) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_2S:
			if (obj->aux2 > 29) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
	}



	//log_e("dx %d dy %d collision \n", dx, dy, obj->collision_state);

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


extern void anim_intro_chase(struct displ_object *obj);
extern void anim_intro_jean(struct displ_object *obj);

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
			if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
				obj->tob->cur_anim_step++;
				if (obj->tob->cur_anim_step > 1)
					obj->tob->cur_anim_step = 0;
				tile_object_show(obj->tob, scr_tile_buffer, true);
				sfx_play_effect(SFX_DOOR, 0);
			}
		} else if (obj->state == 22) {
			obj->tob->cur_anim_step = 0;
			obj->state = 0;
		}
		obj->state++;
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

	if (obj->state == 1) {
		dx = 2;
	} else if (obj->state == 0) {
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
			PATRN_BULLET, ANIM_FALLING_BULLETS, 0, -4, 0, NULL);
		add_bullet(obj->xpos + 8,
			obj->ypos - 8,
			PATRN_BULLET, ANIM_FALLING_BULLETS, 1, -4, 0, NULL);
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
			add_bullet(obj->xpos - 4,
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
	int8_t dx, dy;
	struct spr_sprite_def *sp = obj->spr;

	dx = obj->speed;
	dy = obj->speed;
	if (obj->xpos > dpo_jean.xpos) {
		dx *= -1;
	}
	if (obj->ypos > dpo_jean.ypos) {
		dy *= -1;
	}
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false);
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
		obj->tob->cur_anim_step = 1;
		tile_object_show(obj->tob, scr_tile_buffer, true);
		add_bullet(obj->xpos,
			obj->ypos + 4,
			PATRN_ARROW,
			ANIM_HORIZONTAL_PROJECTILE,
			obj->tob->cur_dir, obj->tob->cur_dir, 8, NULL);
	} else if (obj->state == obj->aux + 10) {
		obj->tob->cur_anim_step = 0;
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
	int8_t dx = 0, dy = 0;

	if (obj->aux == 0) {
		obj->xpos -= obj->aux2;
	} else {
		obj->xpos += obj->aux2;
	}

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
	} else if (obj->state == obj->aux + 10) {
		obj->tob->cur_anim_step = 0;
		tile_object_show(obj->tob, scr_tile_buffer, true);
	} else if (obj->state == 0) {
		tile_object_show(obj->tob, scr_tile_buffer, true);
	}
	if (++obj->state > 60) obj->state = 1;
}

void init_animators()
{
	uint8_t i;

	for (i=0; i< MAX_ANIMATORS; i++) {
		animators[i].page = 6;
	}

	animators[ANIM_LEFT_RIGHT].run = anim_left_right;
	animators[ANIM_LEFT_RIGHT_FLOOR].run = anim_left_right_floor;
	animators[ANIM_UP_DOWN].run = anim_up_down;
	animators[ANIM_JEAN].run = anim_jean;
	animators[ANIM_CYCLE_TILE].run = anim_cycle_tile;
	animators[ANIM_CHASE].run = anim_chase;
	animators[ANIM_CLOSE_DOOR].run = anim_close_door;
	animators[ANIM_SHOOTER_PLANT].run = anim_plant;
	animators[ANIM_FALLING_BULLETS].run = anim_falling_bullets;
	animators[ANIM_WATERDROP].run = anim_waterdrop;
	animators[ANIM_FISH_JUMP].run = anim_fish_jump;
	animators[ANIM_SPLASH].run = anim_splash;
	animators[ANIM_GHOST].run = anim_ghost;
	animators[ANIM_FIREBALL].run = anim_up_down_bounded;
	animators[ANIM_ARCHER_SKELETON].run = anim_archer_skeleton;
	animators[ANIM_HORIZONTAL_PROJECTILE].run = anim_horizontal_projectile;
	animators[ANIM_GARGOLYNE].run = anim_gargolyne;
	animators[ANIM_LEFT_RIGHT_BOUNDED].run = anim_left_right_bounded;

	animators[ANIM_INTRO_CHASE].page = 7;
	animators[ANIM_INTRO_CHASE].run = anim_intro_chase;
	animators[ANIM_INTRO_JEAN].page = 7;
	animators[ANIM_INTRO_JEAN].run = anim_intro_jean;
}
