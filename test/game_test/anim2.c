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

	spr_animate(sp, dx, dy);
	spr_set_pos(sp, obj->xpos, obj->ypos);
	spr_update(sp);

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
	if (sp->cur_anim_step == 2 && sp->anim_ctr == 0 && dpo->aux2 < dpo->aux) {
		obj->aux2++;
		add_bullet(obj->xpos,
			obj->ypos + 24,
			PATRN_SCYTHE, ANIM_SCYTHE, 0, 1, 0, obj);
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
	dy = 2;

	if (obj->state == 0) {
		sp->anim_ctr_treshold = 1;
		obj->state = 1;
		if (obj->xpos > dpo_jean.xpos)
			obj->aux *= -1;
	}

	phys_detect_tile_collisions(obj, scr_tile_buffer, dx, dy, false);
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
	uint8_t x;
	struct tile_object *to = dpo->tob;
	uint16_t offset_bottom = to->x/8 + to->y/8 * 32 + (to->ts->frame_h - 1) * 32;
	uint16_t offset_top = to->x/8 + to->y/8 * 32;

	/** cup has been picked up **/
	if (game_state.cup_picked_up) {
		clear_bullets();
		add_explosion(obj->xpos, obj->ypos, ANIM_EXPLOSION);
		tile_object_hide(dpo->tob, scr_tile_buffer, true);
		list_del(&obj->list);
		return;
	}

	if (obj->state == STATE_MOVING_UP) {
		if (obj->tob->cur_anim_step == 1 && obj->aux == 2) {
			obj->tob->cur_anim_step = 0;
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 0, 3, NULL);
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 1, 3, NULL);
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 2, 3, NULL);
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		} else if (obj->tob->cur_anim_step == 2  && obj->aux == 5) {
			for (x = 0; x < to->ts->frame_w; x++) {
				vdp_write(vdp_base_names_grp1 + offset_bottom + x, 0);
			}
			obj->tob->y-=4;
			obj->ypos-=4;
			if (obj->tob->y < dpo->min) {
				obj->state = STATE_MOVING_DOWN;
			}
			obj->tob->cur_anim_step = 1;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		} else if (obj->tob->cur_anim_step == 1 && obj->aux == 5) {
			obj->tob->cur_anim_step = 2;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		} else if (obj->tob->cur_anim_step == 0 && obj->aux == 2) {
			obj->tob->cur_anim_step = 1;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		}
	} else if (obj->state == STATE_MOVING_DOWN) {
		if (obj->tob->cur_anim_step == 1 && obj->aux == 2) {
			obj->tob->cur_anim_step = 0;
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 0, 3, NULL);
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 1, 3, NULL);
			add_bullet(obj->xpos - 8,
				obj->ypos + 12,
				PATRN_SMALL_BULLET, ANIM_SATAN_BULLETS, 0, 2, 3, NULL);
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		} else if (obj->tob->cur_anim_step == 1 && obj->aux == 5) {
			for (x = 0; x < to->ts->frame_w; x++) {
				vdp_write(vdp_base_names_grp1 + offset_top + x, 0);
			}
			obj->tob->y+=4;
			obj->ypos += 4;
			if (obj->tob->y > dpo->max) {
				obj->state = STATE_MOVING_UP;
			}
			obj->tob->cur_anim_step = 2;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		} else if (obj->tob->cur_anim_step == 2 && obj->aux == 5) {
			obj->tob->cur_anim_step = 1;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		}  else if (obj->tob->cur_anim_step == 0 && obj->aux == 2) {
			obj->tob->cur_anim_step = 1;
			tile_object_show(dpo->tob, scr_tile_buffer, true);
		}
	}

	if (obj->tob->cur_anim_step > 2)
		obj->tob->cur_anim_step = 1;
	if (obj->aux++ > 5)
		obj->aux = 0;
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

	if (obj->xpos < 4 || obj->ypos < 8) {
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
	struct tile_object *to = dpo->tob;
	uint16_t offset_bottom = to->x/8 + to->y/8 * 32 + (to->ts->frame_h - 1) * 32;
	uint16_t offset_top = to->x/8 + to->y/8 * 32;

	if (obj->state == STATE_MOVING_UP) {
		if (obj->tob->cur_anim_step > 0) {
			tile_object_show(obj->tob, scr_tile_buffer, true);
			obj->tob->cur_anim_step--;
		} else {
			vdp_write(vdp_base_names_grp1 + offset_bottom, 0);
			vdp_write(vdp_base_names_grp1 + offset_bottom + 1, 0);
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
	if (obj->state++ < 60) {
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
