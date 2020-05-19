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

#include "logic.h"
#include "scene.h"

struct game_state_t game_state;

void init_game_state()
{
        sys_memset(&game_state, 0, sizeof(game_state));

	game_state.jean_x = 100;
	game_state.jean_y = 192 - 64;
	game_state.room = ROOM_GRAVEYARD;
	game_state.live_cnt = GAME_MAX_LIVES;
	game_state.cross_cnt = 0;
	game_state.templar_ct = 0;
	game_state.death = false;
}

void null_handler(struct displ_object *dpo, uint8_t data)
{
	// empty handler
}


void pickup_heart(struct displ_object *dpo, uint8_t data)
{
        game_state.hearth[data] = 1;
        game_state.live_cnt++;
        remove_tileobject(dpo);
	sfx_play_effect(SFX_PICKUP_ITEM, 0);
}

void pickup_scroll(struct displ_object *dpo, uint8_t data)
{
        game_state.scroll[data] = 1;
        remove_tileobject(dpo);
	sfx_play_effect(SFX_PICKUP_ITEM, 0);
        // TODO: show scroll contents
}

void pickup_cross(struct displ_object *dpo, uint8_t data)
{
        game_state.cross[data] = 1;
        game_state.cross_cnt++;
        remove_tileobject(dpo);
	sfx_play_effect(SFX_PICKUP_ITEM, 0);
}

void checkpoint_handler(struct displ_object *dpo, uint8_t data)
{
        game_state.checkpoint[data] = 1;
        dpo->tob->cur_anim_step = 1;
        update_tileobject(dpo);
	sfx_play_effect(SFX_PICKUP_ITEM, 0);
}


void toggle_handler(struct displ_object *dpo, uint8_t data)
{
        game_state.toggle[data] = 1;
        dpo->tob->cur_anim_step = 1;
        update_tileobject(dpo);
}

void bell_handler(struct displ_object *dpo, uint8_t data)
{
        game_state.bell = true;
        dpo->tob->cur_anim_step = 1;
        update_tileobject(dpo);
}

void crosswitch_handler(struct displ_object *dpo, uint8_t data)
{
        if (!game_state.cross_switch_enable)
                return;

        game_state.cross_switch_enable = false;
        if (game_state.cross_switch) {
                dpo->tob->cur_anim_step = 0;
                game_state.cross_switch = false;
        } else {
                dpo->tob->cur_anim_step = 1;
                game_state.cross_switch = true;
        }
        update_tileobject(dpo);
}

void trigger_handler(struct displ_object *dpo, uint8_t data)
{
        if(game_state.door_trigger)
                return;
        game_state.door_trigger = true;
}
