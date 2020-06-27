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
