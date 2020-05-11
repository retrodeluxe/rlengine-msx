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

#include "anim.h"
#include "logic.h"
#include "scene.h"

struct animator animators[MAX_ANIMATORS];

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

void check_and_change_room()
{
// 	bool change = false;
// 	if (dpo_monk.xpos > 240) {
// 		dpo_monk.xpos = 0;
// 		game_state.map_x+=32;
// 		change = true;
// 	} else if (dpo_monk.xpos == 0) {
// 		dpo_monk.xpos = 240;
// 		game_state.map_x-=32;
// 		change = true;
// 	}
// 	if (dpo_monk.ypos > 192 - 32) {
// 		dpo_monk.ypos = 0;
// 		game_state.map_y+=22;
// 		change = true;
// 	} else if (dpo_monk.ypos < - 32) {
// 		dpo_monk.ypos = 192 - 32 - 32;
// 		game_state.map_y-=22;
// 		change = true;
// 	}
// 	if (change) {
// 		dpo_monk.state = STATE_ONGROUND;
// 		spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
// 		// TODO :: load_room();
// 	}
}

void anim_jean(struct displ_object *obj)
{
	static uint8_t jump_ct = 0;
	int8_t dx, dy, x, y;

	#define CROUCH_OFFSET 8

	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	dx = 0;
	dy = 0;

	x = (sp->planes[0]).x;
	y = (sp->planes[0]).y;

	if (obj->state == STATE_JUMPING) {
		jump_ct++;
		if (is_colliding_down(obj)) {
			jump_ct = 0;
			if (sp->cur_state == JANE_STATE_RIGHT_JUMP) {
				sp->cur_state = JANE_STATE_RIGHT;
				obj->state = STATE_MOVING_RIGHT;
			} else if (sp->cur_state == JANE_STATE_LEFT_JUMP) {
				sp->cur_state = JANE_STATE_LEFT;
				obj->state = STATE_MOVING_LEFT;
			}
		} else {
			dy = jump_ct < 10 ? -4 : 4;
		}
	}

	if (stick == STICK_LEFT) {
		dx = -2;
		if (obj->state == STATE_JUMPING) {
			sp->cur_state = JANE_STATE_LEFT_JUMP;
		} else {
			obj->state = STATE_MOVING_LEFT;
			sp->cur_state = JANE_STATE_LEFT;
		}
	} else if (stick == STICK_RIGHT) {
		dx = 2;
		if (obj->state == STATE_JUMPING) {
			sp->cur_state = JANE_STATE_RIGHT_JUMP;
		} else {
			obj->state = STATE_MOVING_RIGHT;
			sp->cur_state = JANE_STATE_RIGHT;
		}
	} else if (stick == STICK_DOWN && obj->state != STATE_JUMPING) {
		dx = 0;
		if (sp->cur_state == JANE_STATE_RIGHT) {
			sp->cur_state = JANE_STATE_RIGHT_CROUCH;
		} else if (sp->cur_state == JANE_STATE_LEFT) {
			sp->cur_state = JANE_STATE_LEFT_CROUCH;
		}
		obj->state = STATE_CROUCHING;
	} else if (stick == STICK_DOWN_LEFT && obj->state != STATE_JUMPING) {
		dx = -2;
		sp->cur_state = JANE_STATE_LEFT_CROUCH;
		obj->state = STATE_CROUCHING;
	} else if (stick == STICK_DOWN_RIGHT && obj->state != STATE_JUMPING) {
		dx = 2;
		sp->cur_state = JANE_STATE_RIGHT_CROUCH;
		obj->state = STATE_CROUCHING;

	} else if (stick == STICK_CENTER) {
		if (sp->cur_state == JANE_STATE_LEFT_CROUCH) {
			sp->cur_state = JANE_STATE_LEFT;
		} else if (sp->cur_state == JANE_STATE_RIGHT_CROUCH) {
			sp->cur_state = JANE_STATE_RIGHT;
		}
		if (obj->state == STATE_CROUCHING) {
			obj->state = STATE_IDLE;
		} else if (obj->state == STATE_JUMPING) {
			// no changes
		} else if (obj->state == STATE_FALLING) {
			// no changes
		} else {
			obj->state = STATE_IDLE;
		}
	}

	if (trigger && obj->state != STATE_CROUCHING) {
		if (sp->cur_state == JANE_STATE_RIGHT) {
			sp->cur_state = JANE_STATE_RIGHT_JUMP;
		} else if (sp->cur_state == JANE_STATE_LEFT) {
			sp->cur_state = JANE_STATE_LEFT_JUMP;
		}
		obj->state = STATE_JUMPING;
	}

	/** handle fall **/

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy + 4);

	if (obj->state != STATE_JUMPING && !is_colliding_down(obj)) {
		obj->state = STATE_FALLING;
		if (sp->cur_state == JANE_STATE_RIGHT
			|| sp->cur_state == JANE_STATE_RIGHT_CROUCH) {
			sp->cur_state = JANE_STATE_RIGHT_JUMP;
		} else if (sp->cur_state == JANE_STATE_LEFT
			|| sp->cur_state == JANE_STATE_LEFT_CROUCH) {
			sp->cur_state = JANE_STATE_LEFT_JUMP;
		}
	}

	if (obj->state == STATE_FALLING) {
		if (is_colliding_down(obj)) {
			if (sp->cur_state == JANE_STATE_RIGHT_JUMP) {
				sp->cur_state = JANE_STATE_RIGHT;
				obj->state = STATE_MOVING_RIGHT;
			} else if (sp->cur_state == JANE_STATE_LEFT_JUMP) {
				sp->cur_state = JANE_STATE_LEFT;
				obj->state = STATE_MOVING_LEFT;
			}
		} else {
			dy = 4;
		}
	}

	/** handle collisions and update sprite **/

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy);

	if (obj->state != STATE_IDLE) {
		sp->anim_ctr++;
		if (sp->anim_ctr > sp->anim_ctr_treshold) {
			sp->cur_anim_step++;
			sp->anim_ctr = 0;
		}
		if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
			sp->cur_anim_step = 0;
	}

	if (!is_colliding_x(obj)) {
    		obj->xpos += dx;
		x += dx;
	}
	if (!is_colliding_y(obj)) {
		obj->ypos += dy;
		y += dy;
	}

	if (obj->state == STATE_CROUCHING) {
		/* crouching requires temporary offset to jean sprite */
		spr_set_pos(sp, x, y + CROUCH_OFFSET);
		spr_update(sp);
		/* restore pos */
		spr_set_pos(sp, x, y);
	} else {
		spr_set_pos(sp, x, y);
		spr_update(sp);
	}
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
			obj->state = STATE_MOVING_RIGHT;
			obj->visible = true;
			spr_show(obj->spr);
			return;
		case STATE_OFF_SCREEN_DELAY_1S:
			if (game_state.templar_delay > 30) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_2S:
			if (game_state.templar_delay > 60) {
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
