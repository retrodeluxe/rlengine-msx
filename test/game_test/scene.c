#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "displ.h"
#include "phys.h"
#include "list.h"

#include "gen/game_test_tiles_ext.h"
#include "gen/game_test_sprites_ext.h"
#include "gen/game_test_maps.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"

struct tile_set tileset_map1;
struct tile_set tileset_map2;
struct tile_set tileset_map3;
struct tile_set tileset_map4;
struct tile_set tileset_map5;

struct tile_set tileset[TILE_MAX];
struct tile_object tileobject[31];

struct spr_pattern_set spr_pattern[PATRN_MAX];
struct spr_sprite_def enemy_sprites[31];
struct spr_sprite_def monk_sprite;

struct displ_object display_object[32];
struct displ_object dpo_arrow;
struct displ_object dpo_bullet[2];
struct displ_object dpo_monk;

extern struct list_head *elem;
extern struct displ_object *dpo;

struct list_head display_list;

struct map_object_item *map_object;

uint8_t scr_tile_buffer[768];
uint8_t spr_ct, tob_ct;

void find_room_data()
{
	uint8_t pos = game_state.map_x / 32 + (game_state.map_y / 22 * 5);
	map_object = (struct map_object_item *) object_index[pos];
        log_e("find room pos %x\n", map_object);
}


static void free_patterns()
{
	uint8_t i;

	for (i = 0; i < PATRN_MAX; i++)
		spr_pattern[i].allocated = false;

	for (i = 0; i < TILE_MAX; i++) {
		tile_set_vfree(&tileset[i]);
	}
}

static void add_tileobject(struct displ_object *dpo, uint8_t objidx, enum tile_sets_t tileidx)
{
	sys_set_rom();
	tile_set_valloc(&tileset[tileidx]);
	sys_set_bios();
	tileobject[objidx].x = map_object->x;
	tileobject[objidx].y = map_object->y;
	tileobject[objidx].cur_dir = 1;
	tileobject[objidx].cur_anim_step = 0;
	tileobject[objidx].ts = &tileset[tileidx];
	tileobject[objidx].idx = 0;
	dpo->type = DISP_OBJECT_TILE;
	dpo->tob = &tileobject[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
	tob_ct++;
}

void remove_tileobject(struct displ_object *dpo)
{
	list_del(&dpo->list);
	tile_object_hide(dpo->tob, scr_tile_buffer, true);
}

void update_tileobject(struct displ_object *dpo)
{
	tile_object_show(dpo->tob, scr_tile_buffer, true);
}


static void add_sprite(struct displ_object *dpo, uint8_t objidx, enum spr_patterns_t pattidx)
{
	sys_set_rom();
	spr_valloc_pattern_set(&spr_pattern[pattidx]);
	spr_init_sprite(&enemy_sprites[objidx], &spr_pattern[pattidx]);
	sys_set_bios();
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_set_pos(&enemy_sprites[objidx], map_object->x, map_object->y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_ct++;
}

void load_room()
{
	uint8_t i, id;
	spr_ct = 0, tob_ct = 0;

	vdp_screen_disable();
	map_inflate_screen(map, scr_tile_buffer, game_state.map_x, game_state.map_y);

	spr_init();
	free_patterns();
	phys_clear_tile_collision_handlers();
	sys_set_rom();
	spr_valloc_pattern_set(&spr_pattern[PATRN_MONK]);
	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	sys_set_bios();
	INIT_LIST_HEAD(&display_list);
	find_room_data();
	for (dpo = display_object, i = 0; map_object->type != 255 ; i++, dpo++) {
		log_e("dpo %d type : %d\n", i ,map_object->type);
		if (map_object->type == ACTIONITEM) {
			if (map_object->object.actionitem.type == TYPE_SCROLL) {
				id = map_object->object.actionitem.action_id;
				if (game_state.scroll[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_SCROLL);
					phys_set_tile_collision_handler(dpo, pickup_scroll, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_TOGGLE) {
				id = map_object->object.actionitem.action_id;
				if (game_state.toggle[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
					phys_set_tile_collision_handler(dpo, toggle_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_CROSS) {
				id = map_object->object.actionitem.action_id;
				if (game_state.cross[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_CROSS);
					add_animator(dpo, ANIM_CYCLE_TILE);
					phys_set_tile_collision_handler(dpo, pickup_cross, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_TELETRANSPORT) {
				// TODO: just go to the other one
				add_tileobject(dpo, tob_ct, TILE_TELETRANSPORT);
			} else if (map_object->object.actionitem.type == TYPE_HEART) {
				id = map_object->object.actionitem.action_id;
				if (game_state.hearth[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_HEART);
					add_animator(dpo, ANIM_CYCLE_TILE);
					phys_set_tile_collision_handler(dpo, pickup_heart, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_CHECKPOINT) {
				id = map_object->object.actionitem.action_id;
				if (game_state.checkpoint[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_CHECKPOINT);
					phys_set_tile_collision_handler(dpo, checkpoint_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_CHECKPOINT);
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_SWITCH) {
				game_state.cross_switch_enable = true;
				add_tileobject(dpo, tob_ct, TILE_SWITCH);
				phys_set_tile_collision_handler(dpo, crosswitch_handler, 0);
				if(game_state.cross_switch) {
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_CUP) {
				// TODO: end game sequence
				add_tileobject(dpo, tob_ct, TILE_CUP);
			} else if (map_object->object.actionitem.type == TYPE_TRIGGER) {
				// TODO
				// invisible, but needs collision detection.
			} else if (map_object->object.actionitem.type == TYPE_BELL) {
				if (game_state.bell == 0) {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					phys_set_tile_collision_handler(dpo, bell_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					dpo->tob->cur_anim_step = 1;
				}
			} else {
				map_object++;
				continue;
			}
			map_object++;
		} else if (map_object->type == STATIC) {
			if (map_object->object.static_.type == TYPE_DRAGON) {
				add_tileobject(dpo, tob_ct, TILE_DRAGON);
				// here there is some nice animation to do
			} else if (map_object->object.static_.type == TYPE_LAVA) {
				add_tileobject(dpo, tob_ct, TILE_LAVA);
				// also nice animation to do here
			} else if (map_object->object.static_.type == TYPE_SPEAR) {
				add_tileobject(dpo, tob_ct, TILE_SPEAR);
			} else if (map_object->object.static_.type == TYPE_WATER) {
				add_tileobject(dpo, tob_ct, TILE_WATER);
				//add_animator(dpo, ANIM_CYCLE_TILE);
			}
			map_object++;
		} else if (map_object->type == GHOST) {
			add_sprite(dpo, spr_ct, PATRN_GHOST);
			add_animator(dpo, ANIM_STATIC);
			map_object++;
		} else if (map_object->type == ROPE) {
			map_object++;
		} else if (map_object->type == DOOR) {
			// Some of these will show depending on the game_state
			map_object++;
		} else if (map_object->type == SHOOTER) {
			if (map_object->object.shooter.type == TYPE_FLUSH) {
				add_sprite(dpo, spr_ct, PATRN_FISH);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.shooter.type == TYPE_LEAK) {
				add_sprite(dpo, spr_ct, PATRN_WATERDROP);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.shooter.type == TYPE_GARGOYLE) {
				add_tileobject(dpo, tob_ct, TILE_GARGOLYNE);
			} else if (map_object->object.shooter.type == TYPE_ARCHER) {
				add_tileobject(dpo, tob_ct, TILE_ARCHER_SKELETON);
			} else if (map_object->object.shooter.type == TYPE_PLANT) {
				add_tileobject(dpo, tob_ct, TILE_PLANT);
			}
			map_object++;
		} else if (map_object->type == BLOCK) {
			add_tileobject(dpo, tob_ct, TILE_CROSS);
			add_animator(dpo, ANIM_CYCLE_TILE);
			map_object++;
		} else if (map_object->type == STEP) {
			// add special collisions

			// need to see what to do with these ones
			map_object++;
		} else if (map_object->type == MOVABLE) {
			if (map_object->object.movable.type == TYPE_TEMPLAR) {
				add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_BAT) {
				add_sprite(dpo, spr_ct, PATRN_BAT);
				//add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_SPIDER) {
				add_sprite(dpo, spr_ct, PATRN_SPIDER);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_RAT) {
				add_sprite(dpo, spr_ct, PATRN_RAT);
				//add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_WORM) {
				add_sprite(dpo, spr_ct, PATRN_WORM);
				//add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_PRIEST) {
				add_tileobject(dpo, tob_ct, TILE_PRIEST);
			} else if (map_object->object.movable.type == TYPE_FLY) {
				add_sprite(dpo, spr_ct, PATRN_FLY);
				//add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_SKELETON) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_PALADIN) {
				add_sprite(dpo, spr_ct, PATRN_PALADIN);
				//add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_DEATH) {
				// this is a big sprite 32x32 not supported yet
				map_object++;
				continue;
			} else if (map_object->object.movable.type == TYPE_DARK_BAT) {
				add_sprite(dpo, spr_ct, PATRN_DARKBAT);
				//add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_DEMON) {
				add_sprite(dpo, spr_ct, PATRN_DEMON);
				///add_animator(dpo, ANIM_LEFT_RIGHT );
			} else if (map_object->object.movable.type == TYPE_SKELETON_CEIL) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON_CEILING);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_LAVA) {
				add_sprite(dpo, spr_ct, PATRN_FIREBALL);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_SATAN) {
				add_tileobject(dpo, tob_ct, TILE_SATAN);
			} else {
				map_object++;
				continue;
			}
			map_object++;

		} else {
			map_object++;
		}
	}
	INIT_LIST_HEAD(&dpo_monk.animator_list);
	list_add(&animators[ANIM_JOYSTICK].list, &dpo_monk.animator_list);
	list_add(&animators[ANIM_GRAVITY].list, &dpo_monk.animator_list);
	INIT_LIST_HEAD(&dpo_monk.list);
	list_add(&dpo_monk.list, &display_list);
	// show all elements
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			spr_show(dpo->spr);
		} else if (dpo->type == DISP_OBJECT_TILE) {
			log_e("showing dpo\n");
			tile_object_show(dpo->tob, scr_tile_buffer, false);
		}
	}
	vdp_copy_to_vram(scr_tile_buffer, vdp_base_names_grp1, 704);

	vdp_screen_enable();
}



void init_monk()
{
	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	dpo_monk.xpos = 100;
	dpo_monk.ypos = 192 - 64;
	dpo_monk.vy = 0;
	dpo_monk.type = DISP_OBJECT_SPRITE;
	dpo_monk.state = STATE_ONGROUND;
	dpo_monk.spr = &monk_sprite;
	dpo_monk.collision_state = 0;
	spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
}

void init_resources()
{
	uint8_t i;
	tile_init();

	sys_set_rom();
	/** initialize static tile sets for map data */
	INIT_TILE_SET(tileset_map1, maptiles1);
	INIT_TILE_SET(tileset_map2, maptiles2);
	INIT_TILE_SET(tileset_map3, maptiles3);
	INIT_TILE_SET(tileset_map4, maptiles4);
	INIT_TILE_SET(tileset_map5, maptiles5);

	/** allocate static tiles for map */
	tile_set_valloc(&tileset_map1);
	tile_set_valloc(&tileset_map2);
	tile_set_valloc(&tileset_map3);

	/** fixed index allocations for map consistency **/
	tile_set_to_vram(&tileset_map4, 126);
	tile_set_to_vram(&tileset_map5, 126 + 32);

	/** initialize dynamic tile sets */
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SCROLL], scroll, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CHECKPOINT], checkpoint, 2, 3, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CROSS], cross, 2, 2, 4, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_HEART], hearth, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_BELL], bell, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SWITCH], crosswitch, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TOGGLE], toggle, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TELETRANSPORT], portal, 2, 3, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CUP], cup, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_DRAGON], dragon, 11, 5, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_LAVA], lava, 1, 1, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SPEAR], spear, 1, 1, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_WATER], water, 2, 1, 1, 16);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SATAN], satan, 4, 6, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_ARCHER_SKELETON], archer_skeleton, 2, 3, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_GARGOLYNE], gargolyne, 2, 2, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PLANT], plant, 2, 2, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PRIEST], priest, 2, 3, 1, 2);

	/** initialize sprite pattern sets */
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_BAT], SPR_SIZE_16x16, 1, 1, 2, bat);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_RAT], SPR_SIZE_16x16, 1, 2, 2, rat);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SPIDER], SPR_SIZE_16x16, 1, 1, 2, spider);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_MONK], SPR_SIZE_16x32, 1, 2, 3, monk1);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_TEMPLAR], SPR_SIZE_16x32, 1, 2, 2, templar);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_WORM], SPR_SIZE_16x16, 1, 2, 2, worm);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SKELETON], SPR_SIZE_16x32, 1, 2, 2, skeleton);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_PALADIN], SPR_SIZE_16x32, 1, 2, 2, paladin);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_GUADANYA], SPR_SIZE_16x16, 1, 1, 4, guadanya);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_GHOST], SPR_SIZE_16x16, 1, 2, 2, ghost);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_DEMON], SPR_SIZE_16x32, 1, 2, 2, demon);
//	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_DEATH], SPR_SIZE_32x32, 1, 2, 2, death);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_DARKBAT], SPR_SIZE_16x16, 1, 2, 2, darkbat);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_FLY], SPR_SIZE_16x16, 1, 2, 2, fly);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SKELETON_CEILING], SPR_SIZE_16x32, 1, 2, 2, skeleton_ceiling);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_FISH], SPR_SIZE_16x16, 1, 1, 2, fish);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_FIREBALL], SPR_SIZE_16x16, 1, 1, 2, fireball);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_WATERDROP], SPR_SIZE_16x16, 1, 1, 3, waterdrop);
	sys_set_bios();

	for (i = 1; i < 76; i++)
		phys_set_colliding_tile(i);

	phys_clear_colliding_tile(16); // step brown
	phys_clear_colliding_tile(38); // step white
	phys_set_down_colliding_tile(16);
	phys_set_down_colliding_tile(38);

        init_monk();
}
