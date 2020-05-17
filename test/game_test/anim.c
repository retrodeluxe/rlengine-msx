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


struct animator animators[MAX_ANIMATORS];

extern struct displ_object dpo_jean;

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

bool change_room()
{
	bool change = false;

	game_state.jean_x = dpo_jean.xpos;
	game_state.jean_y = dpo_jean.ypos;

	if (dpo_jean.xpos > 239) {
		game_state.jean_x = 0;
		game_state.room += 1;
		change = true;

	} else if (dpo_jean.xpos == 0) {
		game_state.jean_x = 240;
		game_state.room -= 1;
		change = true;
	}
	if (dpo_jean.ypos > 192 - 32) {
		game_state.jean_y = 0;
		game_state.room += 5;
		change = true;

	} else if (dpo_jean.ypos < - 32) {
		game_state.jean_y = 192 - 64;
		game_state.room -= 5;
		change = true;
	}
	return change;
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

	// FIXME: Need to update plane colors when updating the sprite as well
	spr_update(sp);
}

void anim_jean(struct displ_object *obj)
{
	static uint8_t jump_ct = 0, fall_ct = 0;
	static uint8_t death_ct = 0;
	static int8_t dy_8 = 0;
	int8_t dx, dy;
	uint8_t x, y;

	#define CROUCH_OFFSET 8

	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	dx = 0;
	dy = 0;

	x = (sp->planes[0]).x;
	y = (sp->planes[0]).y;

	switch(obj->state) {
		case STATE_COLLISION:
			obj->state = STATE_DEATH;
			sfx_play_effect(SFX_DEATH, 0);
			break;
		case STATE_DEATH:
			death_ct++;
			if (death_ct < 20) {
				anim_jean_death(obj);
				return;
			}
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
			if (!is_colliding_down(obj))
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
			if (!is_colliding_down(obj))
				obj->state = STATE_FALLING;
			break;
		case STATE_CROUCHING:
			if (stick == STICK_DOWN ) {
				// stay here
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
			if (!is_colliding_down(obj))
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
			if (!is_colliding_down(obj))
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
			if (is_colliding_down(obj)) {
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
	if (obj->state == STATE_CROUCHING) {
		obj->ypos = obj->ypos + CROUCH_OFFSET;
		phys_detect_tile_collisions(obj, scr_tile_buffer, &dx, &dy);
		obj->ypos = obj->ypos - CROUCH_OFFSET;
	} else {
		phys_detect_tile_collisions(obj, scr_tile_buffer, &dx, &dy);
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
	if ((dy > 0 && !is_colliding_down(obj))
		|| (dy < 0 && !is_colliding_up(obj))) {
		obj->ypos += dy;
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


	log_e("state %d\n", obj->state);
	log_e("dx %d dy %d\n", dx, dy);
	log_e("x %d y %d\n", obj->xpos, obj->ypos);
	log_e("collision %d\n", obj->collision_state);
}

void anim_static(struct displ_object *obj)
{
	// do nothing, just make sure we can display the sprite
}

void anim_cycle_tile(struct displ_object *dpo)
{
        // maybe I can just set the objet to gone.
	if (dpo->state++ == 10) {
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
	}
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, 0);
	dpo_simple_animate(obj, dx, 0);
}

void anim_left_right_floor(struct displ_object *obj)
{
	int8_t dx = 0;
	switch(obj->state) {
		case STATE_MOVING_LEFT:
			dx = -2;
			if (is_colliding_left(obj) || !is_colliding_down(obj)) {
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
	}
	// FIXME: hanging on detect_tile collisions if dy>0
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, 4);
	dpo_simple_animate(obj, dx, 0);
}

void anim_up_down(struct displ_object *obj)
{
	int8_t dy = 0;
	switch(obj->state) {
		case STATE_MOVING_DOWN:
			dy = 2;
			if (is_colliding_down(obj)) {
				obj->state = STATE_MOVING_UP;
				dy = -2;
			}
			break;
		case STATE_MOVING_UP:
			dy = -2;
			if (is_colliding_up(obj)) {
				obj->state = STATE_MOVING_DOWN;
				dy = 2;
			}
			break;
	}
	phys_detect_tile_collisions(obj, scr_tile_buffer, 0, dy);
	dpo_simple_animate(obj, 0, dy);
}

/**
 * Animation for a Templar chasing Jean in different screens
 */
void anim_chase(struct displ_object *obj)
{
	uint8_t dx;

	game_state.templar_delay++;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 2;
			break;
		case STATE_HOPPING_RIGHT:
			// need to handle obstacles or pits
			// with a jump
			break;
		case STATE_OFF_SCREEN:
			if (game_state.templar_delay > 40) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			return;
		case STATE_OFF_SCREEN_DELAY_1S:
			if (game_state.templar_delay > 80) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_2S:
			if (game_state.templar_delay > 120) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
	}
	dpo_simple_animate(obj, dx, 0);
}

void init_animators()
{
	animators[ANIM_LEFT_RIGHT].run = anim_left_right;
	animators[ANIM_LEFT_RIGHT_FLOOR].run = anim_left_right_floor;
	animators[ANIM_UP_DOWN].run = anim_up_down;
	animators[ANIM_STATIC].run = anim_static;
	animators[ANIM_JEAN].run = anim_jean;
	animators[ANIM_CYCLE_TILE].run = anim_cycle_tile;
	animators[ANIM_CHASE].run = anim_chase;
}
