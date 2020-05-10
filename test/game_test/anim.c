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

struct animator animators[7];

void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

void check_and_change_room()
{
	bool change = false;
	if (dpo_monk.xpos > 240) {
		dpo_monk.xpos = 0;
		game_state.map_x+=32;
		change = true;
	} else if (dpo_monk.xpos == 0) {
		dpo_monk.xpos = 240;
		game_state.map_x-=32;
		change = true;
	}
	if (dpo_monk.ypos > 192 - 32) {
		dpo_monk.ypos = 0;
		game_state.map_y+=22;
		change = true;
	} else if (dpo_monk.ypos < - 32) {
		dpo_monk.ypos = 192 - 32 - 32;
		game_state.map_y-=22;
		change = true;
	}
	if (change) {
		dpo_monk.state = STATE_ONGROUND;
		spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
		// TODO :: load_room();
	}
}

/**
 * dpo animate handles collisions prior to animation.
 *   state transitions must be handled by animators.
 */
void dpo_animate(struct displ_object *dpo, int8_t dx, int8_t dy)
{
	int8_t cdx, cdy;
	int16_t xpos, ypos;
	cdx = dx;
	cdy = dy;

	if (dx != 0 || dy != 0) {
		dpo->xpos += dx;
		dpo->ypos += dy;
		phys_detect_tile_collisions(dpo, scr_tile_buffer, dx, dy);
		// corrections on deltas to account for
		// fixed point physics and 8x8 tile collisions
		if (is_colliding_down(dpo)) {
			ypos = (dpo->ypos / 8) * 8;
			cdy = ypos - dpo->ypos + dy; /* ensure there is collision with ground always */
			dpo->ypos = ypos;
		}
		if (dy < 0 && dpo->ypos > 0  &&
			is_colliding_up(dpo)) {
			ypos = (dpo->ypos / 8) * 8 + 6;
		 	cdy = ypos - dpo->ypos + dy;
		 	dpo->ypos = ypos;
		}

		if (!is_colliding_up(dpo) &&
			(is_colliding_left(dpo) || is_colliding_right(dpo))) {
			dpo->xpos -= dx;
			cdx = 0;
		}
		if (cdx != 0 || cdy != 0) {
			spr_animate(dpo->spr, cdx, cdy);
		}
		//log_e("B ypos %d state %d colision %d\n", dpo->ypos, dpo->state, dpo->collision_state);
	}
}

void anim_joystick(struct displ_object *dpo)
{
	if (stick == STICK_LEFT || stick == STICK_UP_LEFT ||
		stick == STICK_DOWN_LEFT) {
			dpo_animate(dpo, -2, 0);
	}
	if (stick == STICK_RIGHT || stick == STICK_UP_RIGHT ||
		stick == STICK_DOWN_RIGHT) {
			dpo_animate(dpo, 2, 0);
	}
	if (stick == STICK_UP || stick == STICK_UP_RIGHT ||
		stick == STICK_UP_LEFT) {
		if (dpo->state == STATE_ONGROUND) {
			dpo_animate(dpo, 0, -1);
			list_add(&animators[ANIM_JUMP].list, &dpo_monk.animator_list);
			dpo->state = STATE_JUMPING;
		}
	}
	if (stick == STICK_DOWN || stick == STICK_DOWN_LEFT ||
		stick == STICK_DOWN_RIGHT) {
	}
	// if (stick)
	//  	log_e("state : %d colisi %d\n", dpo->state, dpo->collision_state);
}

/**
 * jump animation is enabled from joystick and disables itself
 */
void anim_jump(struct displ_object *dpo)
{
	static uint8_t jmp_ct;

	if (dpo->state == STATE_JUMPING) {
		dpo->state = STATE_ONAIR;
		dpo->vy = - 7;
		jmp_ct = 5;
	} else if (dpo->state == STATE_ONAIR) {
		dpo_animate(dpo, 0, dpo->vy);
		if (is_colliding_up(dpo) || dpo->vy >= 0) {
			dpo->state = STATE_ONCEILING;
			dpo->vy = 0;
		} else {
			dpo->vy++;
		}
	} else if (dpo->state == STATE_ONCEILING) {
		dpo->vy = 0;
		dpo_animate(dpo, 0, dpo->vy);
		if (--jmp_ct == 0) {
			dpo->state = STATE_FALLING;
		}
	} else if (dpo->state == STATE_FALLING) {
		dpo_animate(dpo, 0, dpo->vy);
		if (is_colliding_down(dpo)) {
			dpo->state = STATE_LANDING;
		}
	} else if (dpo->state == STATE_LANDING) {
		list_del(&animators[ANIM_JUMP].list);
		dpo->state = STATE_ONGROUND;
	}
	//log_e("jump collision %d state %d\n", dpo->collision_state, dpo->state);
}

void anim_gravity(struct displ_object *dpo)
{
	if (!is_colliding_down(dpo)) {
		dpo_animate(dpo, 0, dpo->vy);
		if(dpo->vy++ > 2)
			dpo->vy = 2;
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
	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, 0);
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


void init_animators()
{
	animators[ANIM_LEFT_RIGHT].run = anim_left_right;
	animators[ANIM_LEFT_RIGHT_FLOOR].run = anim_left_right_floor;
	animators[ANIM_UP_DOWN].run = anim_up_down;
	animators[ANIM_GRAVITY].run = anim_gravity;
	animators[ANIM_STATIC].run = anim_static;
	// animators[3].run = anim_up_down;
	// animators[4].run = anim_drop;
	animators[ANIM_JOYSTICK].run = anim_joystick;
	animators[ANIM_JUMP].run = anim_jump;
	animators[ANIM_CYCLE_TILE].run = anim_cycle_tile;
}
