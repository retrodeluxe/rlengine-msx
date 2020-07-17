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

#pragma CODE_PAGE 7

extern void add_bullet(uint8_t xpos, uint8_t ypos, uint8_t patrn_id,
			uint8_t anim_id, uint8_t state, uint8_t dir,
			uint8_t speed, struct displ_object *parent);
extern void add_tob_bullet(uint8_t xpos, uint8_t ypos, uint8_t tileidx,
			uint8_t anim_id, uint8_t state, uint8_t dir,
			uint8_t speed, struct displ_object *parent);
extern void add_explosion(uint8_t xpos, uint8_t ypos, uint8_t anim_id);
extern void clear_bullets();

/**
 * Simplified animation for a Templar chasing Jean in intro screen
 */
void anim_intro_chase(struct displ_object *obj)
{
	int8_t dx = 0, dy = 0;
	struct spr_sprite_def *sp = obj->spr;

	obj->aux++;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 1;
			break;
		case STATE_OFF_SCREEN:
			if (obj->aux > 20) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			return;
		case STATE_OFF_SCREEN_DELAY_1S:
			if (obj->aux > 39) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_2S:
			if (obj->aux > 59) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
	}

	obj->xpos += dx;

	if (obj->visible) {
		spr_animate(sp, dx, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}

	if (obj->xpos > 254) {
		spr_hide(sp);
		list_del(&obj->list);
	}
}

/**
 * Simplified Jean animation for intro screen
 */
void anim_intro_jean(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	obj->xpos += 1;
	sp->anim_ctr++;

	if (sp->anim_ctr > sp->anim_ctr_treshold) {
		sp->cur_anim_step++;
		sp->anim_ctr = 0;
	}
	if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
		sp->cur_anim_step = 0;

	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);

	if (obj->xpos > 254) {
		spr_hide(sp);
		list_del(&obj->list);
	}
}

/**
 * Animation Death Boss
 */
void anim_death(struct displ_object *obj)
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
	// based on current animation state, throw bullet which is a scythe with
	// own animation
	if (sp->cur_anim_step == 2 && sp->anim_ctr == 0 && obj->aux2 < obj->aux) {
		obj->aux2++;
		add_bullet(obj->xpos,
			obj->ypos + 24,
			PATRN_SCYTHE, ANIM_SCYTHE, 0, 1, 1, obj);
	}

	obj->xpos += dx;
	spr_animate(sp, dx, 0);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);
}

/**
 * animation of scythe bullets, slowly falling with collision detection,
 *  bullets are deleted when reaching screen borders; shouldn't have more than
 *  3 active simultaneously
 */
void anim_scythe(struct displ_object *obj)
{
	int8_t dx, dy;
	struct spr_sprite_def *sp = obj->spr;

	dx = obj->aux;
	dy = 1;

	if (obj->state == 0) {
		sp->anim_ctr_treshold = 1;
		obj->state = 1;
		if (obj->xpos > dpo_jean.xpos)
			obj->aux *= -1;
	}

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false, false);
	if (!is_colliding_down(obj)) {
		obj->ypos += dy;
	}
	obj->xpos += dx;

	if (obj->xpos < 4 || obj->xpos > 235 || obj->ypos > 150) {
		spr_hide(obj->spr);
		list_del(&obj->list);
		obj->state = 255;
		obj->parent->aux2--;
	} else {
		spr_animate(sp, dx, dy);
		spr_set_pos(sp, obj->xpos, obj->ypos);
		spr_update(sp);
	}
}

/**
 * Final Boss animation, big tile object that moves vertically and
 *  spits clusters of 3 bullets
 */
void anim_satan(struct displ_object *obj)
{
	uint8_t x, ty;
	static uint8_t delay = 0;
	struct tile_object *to = obj->tob;
	uint16_t offset_bottom = to->x/8 + to->y/8 * 32 + (to->ts->frame_h - 1) * 32;
	uint16_t offset_top = to->x/8 + to->y/8 * 32;

	if (obj->aux2) {
		obj->aux2--;
		return;
	} else {
		obj->aux2 = 1; // reduce framerate
	}

	/** cup has been picked up **/
	if (game_state.cup_picked_up) {
		clear_bullets();
		phys_clear_colliding_tile_object(obj);
		add_explosion(obj->xpos, obj->ypos, ANIM_EXPLOSION);
		tile_object_hide(obj->tob, scr_tile_buffer, true);
		list_del(&obj->list);
		return;
	}



	if (obj->state == STATE_MOVING_UP) {
		if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->tob->cur_anim_step++;
		} else {
			for (x = 0; x < to->ts->frame_w; x++) {
				vdp_write(vdp_base_names_grp1 + offset_bottom + x, 0);
			}
			ty = obj->ypos / 8;
			ty--;
			obj->ypos = ty * 8;
			obj->tob->y = ty * 8;
			if (obj->aux++ > 3) {
				obj->tob->cur_anim_step = 0;
				// optimize this so that we can do in a single call maybe?
				add_bullet(obj->xpos - 8,
					obj->ypos,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 0, 4, NULL);
				add_bullet(obj->xpos - 8,
					obj->ypos + 8,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 1, 4, NULL);
				add_bullet(obj->xpos - 8,
					obj->ypos + 16,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 2, 4, NULL);
				obj->aux = 0;
			} else {
				obj->tob->cur_anim_step = 1;
			}
			tile_object_show(obj->tob, scr_tile_buffer, true);
		}
		 if (obj->ypos < obj->min) {
			obj->state = STATE_MOVING_DOWN;
			obj->tob->cur_anim_step = 2;
		}
	} else if (obj->state == STATE_MOVING_DOWN) {
		if (obj->tob->cur_anim_step > 0) {
			if (obj->tob->cur_anim_step == 1 && obj->aux++ > 3) {
				obj->tob->cur_anim_step = 0;
				add_bullet(obj->xpos - 8,
					obj->ypos ,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 0, 4, NULL);
				add_bullet(obj->xpos - 8,
					obj->ypos + 8,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 1, 4, NULL);
				add_bullet(obj->xpos - 8,
					obj->ypos + 16,
					PATRN_BULLET, ANIM_SATAN_BULLETS, 0, 2, 4, NULL);
				obj->aux = 0;
			} else {
				obj->tob->cur_anim_step--;
			}
			tile_object_show(obj->tob, scr_tile_buffer, true);
		} else {
			for (x = 0; x < to->ts->frame_w; x++) {
				vdp_write(vdp_base_names_grp1 + offset_top + x, 0);
			}
			ty = obj->ypos / 8;
			ty++;
			obj->ypos = ty * 8;
			obj->tob->y = ty * 8;
			obj->tob->cur_anim_step = 2;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		}
		if (obj->ypos > obj->max) {
			obj->state = STATE_MOVING_UP;
			obj->tob->cur_anim_step = 1;
		}
	}
}

/**
 * Animation of final boss bullets, move linearly in diagonal directions
 *  no tile collisions; dissapear on wall
 */
void anim_satan_bullets(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	int8_t dx = 0, dy = 0;

	dx = obj->aux2;
	if (obj->aux == 0) {
		dy = -1;
	} else if (obj->aux == 2) {
		dy = 1;
	}

	obj->xpos -= dx;
	obj->ypos += dy;

	if (obj->xpos < 4 || obj->ypos < 4) {
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
 * Animation for Dragon Flame, big tileobject with 2 frames
 *  it flames for alternating frames for ~4 seconds then stops and
 *  spits bullets (flames) on the ground in two directions
 */
void anim_dragon_flame(struct displ_object *obj)
{
	obj->state++;
	if (obj->state < 30) {
			if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
				tile_object_show(obj->tob, scr_tile_buffer, true);
				obj->tob->cur_anim_step++;
			} else {
				obj->tob->cur_anim_step = 0;
			}
	} else if (obj->state == 30) {
		// bullets are tileobjects... how to handle those
		add_tob_bullet(obj->xpos - 8,
				obj->ypos + 40,
				TILE_LAVA, ANIM_DRAGON_BULLETS, 0, 0, 1, NULL);
		add_tob_bullet(obj->xpos + 24,
				obj->ypos + 40,
				TILE_LAVA, ANIM_DRAGON_BULLETS, 0, 1, 1, NULL);
		tile_object_hide(obj->tob, scr_tile_buffer, true);
	} else if (obj->state == 60) {
		obj->state = 0;
	}
}

/**
 * Animation of dragon flame bullets, horizontal translation
 */
void anim_dragon_bullets(struct displ_object *obj)
{
	uint8_t offset_before, offset_after;
	int8_t dx;

	offset_before = obj->xpos/8;

	dx = 1;
	if (obj->aux == 0) {
		dx = -1;
	}

	offset_after = (obj->xpos + dx)/8;

	if (offset_before != offset_after) {
		tile_object_hide(obj->tob, scr_tile_buffer, true);
	}

	obj->xpos += dx;
	obj->tob->x += dx;

	if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
		tile_object_show(obj->tob, scr_tile_buffer, true);
		obj->tob->cur_anim_step++;
	} else {
		obj->tob->cur_anim_step = 0;
	}
	if (obj->xpos < 48 || obj->xpos > 250) {
		tile_object_hide(obj->tob, scr_tile_buffer, true);
		list_del(&obj->list);
		obj->state = 255;
	}
}

/**
 * Animation for priests hanging from tree, length of rope
 *  needs to be adjusted as priests move up and down.
 */
void anim_hanging_priest(struct displ_object *obj)
{
	int8_t dy;
	uint8_t ty;
	struct tile_object *to = obj->tob;
	uint16_t offset_bottom = to->x/8 + to->y/8 * 32 + (to->ts->frame_h - 1) * 32;
	uint16_t offset_top = to->x/8 + to->y/8 * 32;

	if (obj->state == STATE_MOVING_UP) {
		if (obj->tob->cur_anim_step > 0) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->tob->cur_anim_step--;
		} else {
			vdp_write(vdp_base_names_grp1 + offset_bottom, 0);
			vdp_write(vdp_base_names_grp1 + offset_bottom + 1, 0);
			*(scr_tile_buffer + offset_bottom) = 0;
			*(scr_tile_buffer + offset_bottom + 1) = 0;
			ty = obj->ypos / 8;
			ty--;
			obj->ypos = ty * 8;
			obj->tob->y = ty * 8;
			obj->tob->cur_anim_step = 3;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		}
		if (obj->ypos < obj->min) {
			obj->state = STATE_MOVING_DOWN;
			obj->tob->cur_dir = 1;
			obj->tob->cur_anim_step = 0;
		}
	} else if (obj->state == STATE_MOVING_DOWN) {
		if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->tob->cur_anim_step++;
		} else {
			vdp_write(vdp_base_names_grp1 + offset_top, 0);
			vdp_write(vdp_base_names_grp1 + offset_top + 1, 180); // rope
			*(scr_tile_buffer + offset_top) = 0;
			*(scr_tile_buffer + offset_top + 1) = 100;
			ty = obj->ypos / 8;
			ty++;
			obj->ypos = ty * 8;
			obj->tob->y = ty * 8;
			obj->tob->cur_anim_step = 0;
			tile_object_show(obj->tob, scr_tile_buffer, true);
		}
		if (obj->ypos > obj->max) {
			obj->state = STATE_MOVING_UP;
			obj->tob->cur_dir = 0;
			obj->tob->cur_anim_step = 3;
		}
	}
}

void anim_explosion(struct displ_object *obj)
{
	if (obj->state++ < 20) {
		if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->tob->cur_anim_step++;
		} else {
			obj->tob->cur_anim_step = 0;
		}
	} else {
		tile_object_hide(obj->tob, scr_tile_buffer, true);
		list_del(&obj->list);
		game_state.red_parchment = true;
	}
}

void anim_red_parchment(struct displ_object *obj)
{
	if (game_state.red_parchment) {
		obj->visible = true;
		tile_object_show(obj->tob, scr_tile_buffer, true);
	}
}

void anim_evil_chamber(struct displ_object *obj)
{
	int8_t dx = 0, dy = 0;
	struct spr_sprite_def *sp = obj->spr;

	obj->aux++;
	switch(obj->state) {
		case STATE_MOVING_RIGHT:
			dx = 2;
			break;
		case STATE_OFF_SCREEN:
			if (obj->aux > 10) {
				obj->state = STATE_MOVING_RIGHT;
				obj->visible = true;
				spr_show(obj->spr);
			}
			return;
		case STATE_OFF_SCREEN_DELAY_1S:
			if (obj->aux > 19) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_2S:
			if (obj->aux > 29) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
		case STATE_OFF_SCREEN_DELAY_3S:
			if (obj->aux > 39) {
				obj->state = STATE_OFF_SCREEN;
			}
			return;
	}

	obj->xpos += dx;

	spr_animate(sp, dx, dy);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);

	if (obj->xpos > 140) {
		game_state.show_parchment = 8; // blue parchment
		game_state.start_bonfire_seq = true;
	}
}

/**
 * Animation for the bonfire ending sequence
 *  jean shakes left and right 3 times then dies.
 */
void anim_jean_bonfire(struct displ_object *obj)
{
	// needs to last a bit longer....
	// and is not showing templar sprites...
	struct spr_sprite_def *sp = obj->spr;
	struct spr_pattern_set *ps = sp->pattern_set;

	if (obj->state == 0) {
		sp->cur_state = JANE_STATE_LEFT_JUMP;
		spr_update(sp);
	} else if (obj->state == 50) {
		sp->cur_state = JANE_STATE_RIGHT_JUMP;
		spr_update(sp);
	} else if (obj->state == 100) {
		sp->cur_state = JANE_STATE_LEFT_JUMP;
		spr_update(sp);
	} else if (obj->state == 150) {
		// doesn't sound because there isn't any music...
		sfx_play_effect(SFX_DEATH, 0);
	} else if (obj->state > 150 && obj->state < 170) {
		sp->cur_state = JANE_STATE_DEATH;
		sp->anim_ctr++;
		if (sp->anim_ctr > sp->anim_ctr_treshold) {
			sp->cur_anim_step++;
			sp->anim_ctr = 0;
		}
		if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
			sp->cur_anim_step = 0;

		spr_update(sp);
	} else if (obj->state == 170) {
		game_state.final_animation = true;
	}
	obj->state++;
}

/**
 * Animation for block crosses apearing sequentially on final boss
 */
// compiler issue, ct2 fails to initialize properly

void anim_block_crosses(struct displ_object *obj)
{
	static uint8_t ct1 = 0;
	static uint8_t ct2 = 0;

	if (ct1++ > 40) {
		if (game_state.cross_cnt > 0) {
			ct2++;
			game_state.cross_cnt--;
			game_state.refresh_score = true;
		}
		ct1 = 0;
	}
	if (ct2 >= obj->aux) {
		if (obj->state == 0) {
			sfx_play_effect(SFX_SHOOT, 0);
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->state++;
		}
		if (obj->state == 12) {
			if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
				tile_object_show(obj->tob, scr_tile_buffer, true);
				obj->tob->cur_anim_step++;
			} else {
				obj->tob->cur_anim_step = 0;
			}
			obj->state = 1;
		}
		obj->state++;
	}
}

/**
 * Animation for crosses with dynamic switch
 */
void anim_cross(struct displ_object *obj)
{
	if (game_state.cross_switch) {
		if(obj->aux == 1)
			obj->visible = true;
		else
			obj->visible = false;
	} else if (!game_state.cross_switch) {
		if(obj->aux == 0)
			obj->visible = true;
		else
			obj->visible = false;
	}
	if (obj->visible) {
		if (obj->state++ == 5) {
			if (obj->tob->cur_anim_step < obj->tob->ts->n_frames) {
				tile_object_show(obj->tob, scr_tile_buffer, true);
				obj->tob->cur_anim_step++;
			} else {
				obj->tob->cur_anim_step = 0;
			}
			obj->state = 0;
		}
	}
}

void anim_templar_bonfire(struct displ_object *obj)
{
	struct spr_sprite_def *sp = obj->spr;
	spr_update(sp);
}
