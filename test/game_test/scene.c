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
#include "pt3.h"
#include "font.h"
#include "sfx.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"
#include "banks.h"

#include "gen/game_test_tiles_ext.h"
#include "gen/game_test_sprites_ext.h"
#include "gen/map_defs.h"

#include <stdlib.h>

#pragma CODE_PAGE 4

/** map tilesets **/
struct tile_set tileset_map[MAP_TILESET_MAX];

/** object tilesets **/
struct tile_set tileset[TILE_MAX];

/** scene primitives **/
struct tile_object tileobject[SCENE_MAX_DPO];
struct spr_sprite_def enemy_sprites[SCENE_MAX_DPO];
struct spr_sprite_def bullet_sprites[SCENE_MAX_BULLET];
struct spr_sprite_def jean_sprite;

// FIXME: there appears to be a buffer overrun in one of this structures,
//        because rearranging them in the wrong order leads to a hang on start

/** scene display objects **/
struct displ_object display_object[SCENE_MAX_DPO];
struct displ_object dpo_bullet[SCENE_MAX_BULLET];

/** main character display object **/
struct displ_object dpo_jean;

/** main display list **/
struct list_head display_list;

/** score panel primitives **/
struct tile_object score;
struct font big_digits;

/** current map object **/
struct map_object_item *map_object;

/** iterators to check for sprite collisions **/
struct list_head *coll_elem;
struct displ_object *coll_dpo;

/** tile object and sprite counters **/
uint8_t spr_ct, tob_ct, bullet_ct;

/** misc flags **/
bool init_bullets;
bool init_fish;

/** current room object data **/
uint8_t *room_objs;

/** sprite definition data */
extern const uint8_t two_step_state[];
extern const uint8_t single_step_state[];
extern const uint8_t three_step_state[];
extern const uint8_t bullet_state[];
extern const uint8_t bat_state[];
extern const uint8_t waterdrop_state[];
extern const uint8_t single_four_state[];
extern const uint8_t spider_state[];
extern const uint8_t archer_state[];
extern const uint8_t jean_state[];

void define_sprite(uint8_t pattidx);

void play_room_music() __nonbanked
{
	pt3_decode();
	sfx_play();
	pt3_play();
}

void stop_music()
{
	sys_irq_unregister(play_room_music);
	pt3_mute();
}

void start_music(uint8_t room)
{
	ascii8_set_data(PAGE_MUSIC);

	switch (room) {
		case ROOM_FOREST:
		case ROOM_GRAVEYARD:
			sys_memcpy(data_buffer, huntloop_song_pt3, huntloop_song_pt3_len);
			break;
		case ROOM_CHURCH_ENTRANCE:
		case ROOM_CHURCH_ALTAR:
		case ROOM_CHURCH_TOWER:
		case ROOM_CHURCH_WINE_SUPPLIES:
		case ROOM_CATACOMBS:
		case ROOM_CATACOMBS_FLIES:
		case ROOM_CATACOMBS_WHEEL:
			sys_memcpy(data_buffer, church_song_pt3, church_song_pt3_len);
			break;
		case ROOM_PRAYER_OF_HOPE:
			sys_memcpy(data_buffer, prayerofhope_song_pt3, prayerofhope_song_pt3_len);
			break;
		case ROOM_CAVE_LAKE:
		case ROOM_CAVE_DRAGON:
		case ROOM_CAVE_GHOST:
		case ROOM_CAVE_TUNNEL:
		case ROOM_HIDDEN_GARDEN:
		case ROOM_HIDDEN_RIVER:
			sys_memcpy(data_buffer, cave_song_pt3, cave_song_pt3_len);
			break;
		case ROOM_EVIL_CHURCH:
		case ROOM_EVIL_CHURCH_2:
		case ROOM_EVIL_CHURCH_3:
			sys_memcpy(data_buffer, hell_song_pt3, hell_song_pt3_len);
			break;
		default:
			break;
	}

	pt3_init(data_buffer, 1);
	sys_irq_register(play_room_music);
}

static void add_tileobject(struct displ_object *dpo, uint8_t objidx, enum tile_sets_t tileidx)
{
	ascii8_set_data(PAGE_DYNTILES);
	tile_set_valloc(&tileset[tileidx]);

	ascii8_set_data(PAGE_MAPOBJECTS);
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
	dpo->visible = true;
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
	define_sprite(pattidx);
	spr_valloc_pattern_set(pattidx);
	spr_init_sprite(&enemy_sprites[objidx], pattidx);

	ascii8_set_data(PAGE_MAPOBJECTS);
	spr_set_pos(&enemy_sprites[objidx], map_object->x, map_object->y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	dpo->collision_state = 0;
	dpo->check_collision = true;

	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_ct++;
}

void add_jean()
{
	define_sprite(PATRN_JEAN);
	spr_valloc_pattern_set(PATRN_JEAN);

	spr_init_sprite(&jean_sprite, PATRN_JEAN);

	dpo_jean.xpos = game_state.jean_x;
	dpo_jean.ypos = game_state.jean_y;
	dpo_jean.type = DISP_OBJECT_SPRITE;
	dpo_jean.state = STATE_IDLE;
	dpo_jean.spr = &jean_sprite;
	dpo_jean.collision_state = 0;
	dpo_jean.check_collision = false;
	spr_set_pos(&jean_sprite, dpo_jean.xpos, dpo_jean.ypos);
	INIT_LIST_HEAD(&dpo_jean.list);
	list_add(&dpo_jean.list, &display_list);
	INIT_LIST_HEAD(&dpo_jean.animator_list);
	add_animator(&dpo_jean, ANIM_JEAN);
}

/**
 * throws a plant bullet to either left (dir = 0) or right (dir = 1)
 */
void add_plant_bullet(uint8_t xpos, uint8_t ypos, uint8_t dir)
{
	uint8_t idx;

	if (init_bullets) {
		define_sprite(PATRN_BULLET);
		spr_valloc_pattern_set(PATRN_BULLET);
		init_bullets = false;
	}

	idx = 0;
	for (idx = 0; idx < SCENE_MAX_BULLET; idx++) {
		if (dpo_bullet[idx].state == 255)
			break;
	}

	if (idx == SCENE_MAX_BULLET)
		return;

	spr_init_sprite(&bullet_sprites[idx], PATRN_BULLET);
	bullet_sprites[idx].cur_anim_step = 0;
	spr_set_pos(&bullet_sprites[idx], xpos + 8 * dir, ypos - 8);

	dpo_bullet[idx].type = DISP_OBJECT_SPRITE;
	dpo_bullet[idx].spr = &bullet_sprites[idx];
	dpo_bullet[idx].xpos = xpos + 8 * dir;
	dpo_bullet[idx].ypos = ypos - 8;
	dpo_bullet[idx].state = dir;
	dpo_bullet[idx].collision_state = 0;
	dpo_bullet[idx].aux = -4;

	INIT_LIST_HEAD(&dpo_bullet[idx].list);
	INIT_LIST_HEAD(&dpo_bullet[idx].animator_list);
	add_animator(&dpo_bullet[idx], ANIM_FALLING_BULLETS);
	list_add(&dpo_bullet[idx].list, &display_list);
	spr_show(dpo_bullet[idx].spr);

}

void add_fish_bullet(uint8_t xpos, uint8_t ypos)
{
	uint8_t idx;

	if (init_fish) {
		define_sprite(PATRN_FISH);
		spr_valloc_pattern_set(PATRN_FISH);
		init_fish = false;
	}

	idx = 0;
	for (idx = 0; idx < SCENE_MAX_BULLET; idx++) {
		if (dpo_bullet[idx].state == 255)
			break;
	}

	if (idx == SCENE_MAX_BULLET)
		return;

	spr_init_sprite(&bullet_sprites[idx], PATRN_FISH);
	bullet_sprites[idx].cur_anim_step = 0;
	spr_set_pos(&bullet_sprites[idx], xpos, ypos - 8);

	dpo_bullet[idx].type = DISP_OBJECT_SPRITE;
	dpo_bullet[idx].spr = &bullet_sprites[idx];
	dpo_bullet[idx].xpos = xpos;
	dpo_bullet[idx].ypos = ypos - 8;
	dpo_bullet[idx].state = 0;
	dpo_bullet[idx].collision_state = 0;
	dpo_bullet[idx].aux = -6;
	dpo_bullet[idx].aux2 = 0;

	INIT_LIST_HEAD(&dpo_bullet[idx].list);
	INIT_LIST_HEAD(&dpo_bullet[idx].animator_list);
	add_animator(&dpo_bullet[idx], ANIM_FISH_JUMP);
	list_add(&dpo_bullet[idx].list, &display_list);
	spr_show(dpo_bullet[idx].spr);
}


inline bool jean_check_collision(struct displ_object *dpo) __nonbanked
{
	if (dpo->type == DISP_OBJECT_SPRITE && dpo->check_collision) {
		if (dpo->xpos < (dpo_jean.xpos + 16) &&
			(dpo->xpos + 16) > dpo_jean.xpos) {
			if (dpo->ypos < (dpo_jean.ypos + 32) &&
				(dpo->ypos + 16) > dpo_jean.ypos) {
				return true;
			}
		}
	}
	return false;
}

void jean_collision_handler() __nonbanked
{
	if (dpo_jean.state != STATE_COLLISION
		&& dpo_jean.state != STATE_DEATH) {
		/** calculate box intersection on all active sprites **/
		list_for_each(coll_elem, &display_list) {
			coll_dpo = list_entry(coll_elem,
				struct displ_object, list);
			if (jean_check_collision(coll_dpo)) {
				 dpo_jean.state = STATE_COLLISION;
				 return;
			}
		}
	}
}

void clear_room() {
	uint8_t i;

	/* clear all sprite attributes and patterns from VRAM but
	   _not_ sprite definitions in RAM */
	spr_clear();

	/* free dynamic tiles, but do not clear patterns from VRAM */
	for (i = 0; i < tob_ct; i++) {
		tile_set_vfree(tileobject[i].ts);
	}

	spr_ct = 0;
	tob_ct = 0;
	bullet_ct = 0;
	init_bullets = true;
	init_fish = true;

	for (i = 0; i < SCENE_MAX_BULLET; i++) {
		dpo_bullet[i].state = 255;
	}
}

/**
 * clean room ephemeral state
 */
void clean_state()
{
	game_state.templar_delay = 0;
	game_state.templar_ct = 0;
}



void load_room(uint8_t room)
{
	uint8_t i, id, type, delay, speed;
	bool add_dpo;

	sys_irq_disable();

	stop_music();

	clear_room();
	clean_state();
	vdp_screen_disable();

	ascii8_set_data(PAGE_MAP);
	map_inflate(map_map_segment_dict[room], map_map_segment[room], scr_tile_buffer, 192, 32);

	init_sfx();

	phys_init();
	init_tile_collisions();

	INIT_LIST_HEAD(&display_list);

	ascii8_set_data(PAGE_MAPOBJECTS);

	//log_e("room : %d\n",room);

	type = 0;
	room_objs = map_object_layer[room];

	for (dpo = display_object, i = 0; type != 255 ; i++, dpo++) {
		ascii8_set_data(PAGE_MAPOBJECTS);
		map_object = (struct map_object_item *) room_objs;
		type = map_object->type;
		if (type == ACTIONITEM) {
			uint8_t action_item_type = map_object->object.actionitem.type;
			// log_e("action_item_type %d\n", action_item_type);
			if (action_item_type == TYPE_SCROLL) {
				id = map_object->object.actionitem.action_id;
				if (game_state.scroll[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_SCROLL);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, pickup_scroll, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_TOGGLE) {
				id = map_object->object.actionitem.action_id;
				if (game_state.toggle[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, toggle_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_CROSS) {
				id = map_object->object.actionitem.action_id;
				if (game_state.cross[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_CROSS);
					add_animator(dpo, ANIM_CYCLE_TILE);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, pickup_cross, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_TELETRANSPORT) {
				// TODO: just go to the other one
				add_tileobject(dpo, tob_ct, TILE_TELETRANSPORT);
			} else if (map_object->object.actionitem.type == TYPE_HEART) {
				id = map_object->object.actionitem.action_id;
				if (game_state.hearth[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_HEART);
					add_animator(dpo, ANIM_CYCLE_TILE);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, pickup_heart, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_CHECKPOINT) {
				id = map_object->object.actionitem.action_id;
				if (game_state.checkpoint[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_CHECKPOINT);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, checkpoint_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_CHECKPOINT);
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_SWITCH) {
				game_state.cross_switch_enable = true;
				add_tileobject(dpo, tob_ct, TILE_SWITCH);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_TRIGGER, crosswitch_handler, 0);
				if(game_state.cross_switch) {
					dpo->tob->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_CUP) {
				// TODO: end game sequence
				add_tileobject(dpo, tob_ct, TILE_CUP);
			} else if (map_object->object.actionitem.type == TYPE_TRIGGER) {
				id = map_object->object.actionitem.action_id;
				add_tileobject(dpo, tob_ct, TILE_INVISIBLE_TRIGGER);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_TRIGGER, trigger_handler, id);
			} else if (map_object->object.actionitem.type == TYPE_BELL) {
				if (!game_state.bell) {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, bell_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					dpo->tob->cur_anim_step = 1;
				}
			} else {
				room_objs += NEXT_OBJECT(struct map_object_actionitem);
				continue;
			}
			room_objs += NEXT_OBJECT(struct map_object_actionitem);
		} else if (map_object->type == STATIC) {
			if (map_object->object.static_.type == TYPE_DRAGON) {
 				// this is crashing, ignore
				//add_tileobject(dpo, tob_ct, TILE_DRAGON);
				// here there is some nice animation to do
			} else if (map_object->object.static_.type == TYPE_LAVA) {
				add_tileobject(dpo, tob_ct, TILE_LAVA);
				// also nice animation to do here
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, spear_handler, 0);
			} else if (map_object->object.static_.type == TYPE_SPEAR) {
				add_tileobject(dpo, tob_ct, TILE_SPEAR);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, spear_handler, 0);
			} else if (map_object->object.static_.type == TYPE_WATER) {
				add_tileobject(dpo, tob_ct, TILE_WATER);
				add_animator(dpo, ANIM_CYCLE_TILE);
			}
			room_objs += NEXT_OBJECT(struct map_object_static);
		} else if (map_object->type == GHOST) {
			add_sprite(dpo, spr_ct, PATRN_GHOST);
			add_animator(dpo, ANIM_STATIC);
			room_objs += NEXT_OBJECT(struct map_object_ghost);
		} else if (map_object->type == ROPE) {
			room_objs += NEXT_OBJECT(struct map_object_rope);
		} else if (map_object->type == DOOR) {
			type = map_object->object.door.type;
			id = map_object->object.door.action_id;
			add_dpo = false;
			if (id == 0) {
				dpo->visible = game_state.door_trigger;
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				add_animator(dpo, ANIM_CLOSE_DOOR);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			} else if (id == 1 && game_state.toggle[0] == 0) {
				add_dpo = true;
			} else if (id == 2 && !game_state.bell) {
				add_tileobject(dpo, tob_ct, TILE_TRAPDOOR);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			} else if (id == 3 && game_state.toggle[1] == 0) {
				// cave lake door
				add_dpo = true;
			} else if (id == 4 && game_state.toggle[2] == 0) {
				add_dpo = true;
			} else if (id == 5) {
				// church tower door
				add_dpo = true;
			}
			if (add_dpo) {
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				if (type == 1) {
					dpo->tob->cur_dir = 2;
					dpo->tob->cur_anim_step = 1;
				}
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			}
			room_objs += NEXT_OBJECT(struct map_object_door);
		} else if (map_object->type == SHOOTER) {
			if (map_object->object.shooter.type == TYPE_FLUSH) {
				delay = map_object->object.shooter.delay;
				add_tileobject(dpo, tob_ct, TILE_SPLASH);
				dpo->visible = false;
				dpo->aux = delay;
				add_animator(dpo, ANIM_SPLASH);
			} else if (map_object->object.shooter.type == TYPE_LEAK) {
				add_sprite(dpo, spr_ct, PATRN_WATERDROP);
				add_animator(dpo, ANIM_WATERDROP);
			} else if (map_object->object.shooter.type == TYPE_GARGOYLE) {
				add_tileobject(dpo, tob_ct, TILE_GARGOLYNE);
			} else if (map_object->object.shooter.type == TYPE_ARCHER) {
				add_tileobject(dpo, tob_ct, TILE_ARCHER_SKELETON);
			} else if (map_object->object.shooter.type == TYPE_PLANT) {
				delay = map_object->object.shooter.delay;
				dpo->aux = delay;
				add_tileobject(dpo, tob_ct, TILE_PLANT);
				add_animator(dpo, ANIM_SHOOTER_PLANT);
			}
			room_objs += NEXT_OBJECT(struct map_object_shooter);
		} else if (map_object->type == BLOCK) {
			add_tileobject(dpo, tob_ct, TILE_CROSS);
			add_animator(dpo, ANIM_CYCLE_TILE);
			room_objs += NEXT_OBJECT(struct map_object_block);
		} else if (map_object->type == STEP) {
			// TODO: Add special collisions
			room_objs += NEXT_OBJECT(struct map_object_step);
		} else if (map_object->type == STAINEDGLASS) {
			add_tileobject(dpo, tob_ct, TILE_STAINED_GLASS);
			room_objs += NEXT_OBJECT(struct map_object_stainedglass);
		} else if (map_object->type == MOVABLE) {
			if (map_object->object.movable.type == TYPE_TEMPLAR) {
				add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
				if (room == ROOM_FOREST ||
					room == ROOM_GRAVEYARD) {
					add_animator(dpo, ANIM_CHASE);
					dpo->state = STATE_OFF_SCREEN;
					dpo->visible = false;
					dpo->spr->cur_state = SPR_STATE_RIGHT;
					if (game_state.templar_ct == 1) {
						dpo->state = STATE_OFF_SCREEN_DELAY_1S;
					} else if (game_state.templar_ct == 2) {
						dpo->state = STATE_OFF_SCREEN_DELAY_2S;
					}
					game_state.templar_ct++;
				}
			} else if (map_object->object.movable.type == TYPE_BAT) {
				add_sprite(dpo, spr_ct, PATRN_BAT);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_SPIDER) {
				add_sprite(dpo, spr_ct, PATRN_SPIDER);
				add_animator(dpo, ANIM_UP_DOWN);
				dpo->state = STATE_MOVING_DOWN;
			} else if (map_object->object.movable.type == TYPE_RAT) {
				add_sprite(dpo, spr_ct, PATRN_RAT);
				add_animator(dpo, ANIM_LEFT_RIGHT_FLOOR);
			} else if (map_object->object.movable.type == TYPE_WORM) {
				add_sprite(dpo, spr_ct, PATRN_WORM);
				add_animator(dpo, ANIM_LEFT_RIGHT_FLOOR);
			} else if (map_object->object.movable.type == TYPE_PRIEST) {
				add_tileobject(dpo, tob_ct, TILE_PRIEST);
			} else if (map_object->object.movable.type == TYPE_FLY) {
				speed = map_object->object.movable.speed;
				add_sprite(dpo, spr_ct, PATRN_FLY);
				add_animator(dpo, ANIM_UP_DOWN);
				dpo->state = STATE_MOVING_DOWN;
				dpo->speed = speed;
			} else if (map_object->object.movable.type == TYPE_SKELETON) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON);
				add_animator(dpo, ANIM_LEFT_RIGHT_FLOOR);
			} else if (map_object->object.movable.type == TYPE_PALADIN) {
				add_sprite(dpo, spr_ct, PATRN_PALADIN);
				add_animator(dpo, ANIM_LEFT_RIGHT_FLOOR);
			// } else if (map_object->object.movable.type == TYPE_DEATH) {
			// 	// this is a big sprite 32x32 not supported yet
			// 	map_object++;
			// 	continue;
			} else if (map_object->object.movable.type == TYPE_DARK_BAT) {
				add_sprite(dpo, spr_ct, PATRN_DARKBAT);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_DEMON) {
				add_sprite(dpo, spr_ct, PATRN_DEMON);
				add_animator(dpo, ANIM_LEFT_RIGHT_FLOOR);
			} else if (map_object->object.movable.type == TYPE_SKELETON_CEIL) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON_CEILING);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_LAVA) {
				add_sprite(dpo, spr_ct, PATRN_FIREBALL);
				add_animator(dpo, ANIM_STATIC);
			} else if (map_object->object.movable.type == TYPE_SATAN) {
				add_tileobject(dpo, tob_ct, TILE_SATAN);
			} else {
				room_objs += NEXT_OBJECT(struct map_object_movable);
				continue;
			}
			room_objs += NEXT_OBJECT(struct map_object_movable);
		}
	}

	add_jean();
	// phys_set_sprite_collision_handler(jean_collision_handler);

	// show all elements
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
			spr_show(dpo->spr);
		} else if (dpo->type == DISP_OBJECT_TILE && dpo->visible) {
			tile_object_show(dpo->tob, scr_tile_buffer, false);
		}
	}


	vdp_copy_to_vram(scr_tile_buffer, vdp_base_names_grp1, 704);
	vdp_screen_enable();
	start_music(room);
}

void init_tile_collisions()
{
	uint8_t i;
	for (i = 1; i < 76; i++)
		phys_set_colliding_tile(i);

	phys_clear_colliding_tile(16); // step brown
	phys_clear_colliding_tile(38); // step white
	phys_set_down_colliding_tile(16);
	phys_set_down_colliding_tile(38);
}

void init_map_tilesets()
{
	tile_init();

	ascii8_set_data(PAGE_MAPTILES);

	INIT_TILE_SET(tileset_map[MAP_TILESET_1], maptiles1);
	INIT_TILE_SET(tileset_map[MAP_TILESET_2], maptiles2);
	INIT_TILE_SET(tileset_map[MAP_TILESET_3], maptiles3)
	INIT_TILE_SET(tileset_map[MAP_TILESET_4], maptiles4);
	INIT_TILE_SET(tileset_map[MAP_TILESET_5], maptiles5);

	tile_set_valloc(&tileset_map[MAP_TILESET_1]);
	tile_set_valloc(&tileset_map[MAP_TILESET_2]);
	tile_set_valloc(&tileset_map[MAP_TILESET_3]);
	tile_set_to_vram(&tileset_map[MAP_TILESET_4], MAP_TILESET_4_POS);
	tile_set_to_vram(&tileset_map[MAP_TILESET_5], MAP_TILESET_5_POS);
}

void show_score_panel()
{
	uint8_t i;
	char snum[3];

	ascii8_set_data(PAGE_DYNTILES);
	tile_set_to_vram_bank(&tileset[TILE_HEART_STATUS], BANK2, 252 - 4);
	tile_set_to_vram_bank(&tileset[TILE_CROSS_STATUS], BANK2, 252 - 8);

	score.y = 192 - 16;
	score.x = 0;
	score.cur_dir = 1;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_HEART_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	score.x = 32;
	score.y = 192 - 16;
	score.cur_dir = 1;
	score.cur_anim_step = 0;
	score.ts = &tileset[TILE_CROSS_STATUS];
	score.idx = 0;

	tile_object_show(&score, scr_tile_buffer, true);

	_itoa(game_state.live_cnt, snum, 10);
	snum[2] = '~';

	font_vprint(&big_digits, 2, 22, snum);

	_itoa(game_state.cross_cnt, snum, 10);
	snum[2] = '~';

	font_vprint(&big_digits, 6, 22, snum);
}

void init_resources()
{
	init_map_tilesets();
	spr_init();

	/** initialize dynamic tile sets **/
	ascii8_set_data(PAGE_DYNTILES);

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
	INIT_DYNAMIC_TILE_SET(tileset[TILE_WATER], water, 1, 1, 8, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SATAN], satan, 4, 6, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_ARCHER_SKELETON], archer_skeleton, 2, 3, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_GARGOLYNE], gargolyne, 2, 2, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PLANT], plant, 2, 2, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_PRIEST], priest, 2, 3, 1, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_DOOR], door, 1, 4, 2, 2);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TRAPDOOR], trapdoor, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_INVISIBLE_TRIGGER], invisible_trigger, 1, 4, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_CROSS_STATUS], cross_status, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_HEART_STATUS], hearth_status, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_STAINED_GLASS], stainedglass, 6, 6, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_SPLASH], splash, 2, 1, 3, 1);

	/** copy numeric font to vram **/
	ascii8_set_data(PAGE_INTRO);
	init_font(&big_digits, font_big_digits_tile, font_big_digits_tile_color, 10, 2,
		FONT_NUMERIC, 10, 1, 2);

	font_to_vram_bank(&big_digits, BANK2, 224);
}

void init_sfx()
{
	/** copy over sfx to ram **/
	ascii8_set_data(PAGE_MUSIC);
	sfx_setup(abbaye_sfx_afb);
}

void define_sprite(uint8_t pattidx)
{
	uint16_t size;
	uint8_t frames;

	ascii8_set_data(PAGE_SPRITES);
	switch(pattidx) {
		case PATRN_BAT:
			spr_define_pattern_set(PATRN_BAT, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_BAT, bat, bat_color);
			break;
		case PATRN_RAT:
			spr_define_pattern_set(PATRN_RAT, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_RAT, rat, rat_color);
			break;
		case PATRN_SPIDER:
			spr_define_pattern_set(PATRN_SPIDER, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_SPIDER, spider, spider_color);
			break;
		case PATRN_JEAN:
			spr_define_pattern_set(PATRN_JEAN, SPR_SIZE_16x32, 1, 7,
				jean_state);
			spr_copy_pattern_set(PATRN_JEAN, monk1, monk1_color);
			break;
		case PATRN_TEMPLAR:
			spr_define_pattern_set(PATRN_TEMPLAR, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_TEMPLAR, templar, templar_color);
			break;
		case PATRN_WORM:
			spr_define_pattern_set(PATRN_WORM, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_WORM, worm, worm_color);
			break;
		case PATRN_SKELETON:
			spr_define_pattern_set(PATRN_SKELETON, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_SKELETON, skeleton, skeleton_color);
			break;
		case PATRN_PALADIN:
			spr_define_pattern_set(PATRN_PALADIN, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_PALADIN, paladin, paladin_color);
			break;
		case PATRN_GUADANYA:
			spr_define_pattern_set(PATRN_GUADANYA, SPR_SIZE_16x16, 1, 1,
				single_four_state);
			spr_copy_pattern_set(PATRN_GUADANYA, guadanya, guadanya_color);
			break;
		case PATRN_GHOST:
			spr_define_pattern_set(PATRN_GHOST, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_GHOST, ghost, ghost_color);
			break;
		case PATRN_DEMON:
			spr_define_pattern_set(PATRN_DEMON, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_DEMON, demon, demon_color);
			break;
		case PATRN_DARKBAT:
			spr_define_pattern_set(PATRN_DARKBAT, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_DARKBAT, darkbat, darkbat_color);
			break;
		case PATRN_FLY:
			spr_define_pattern_set(PATRN_FLY, SPR_SIZE_16x16, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_FLY, fly, fly_color);
			break;
		case PATRN_SKELETON_CEILING:
			spr_define_pattern_set(PATRN_SKELETON_CEILING, SPR_SIZE_16x32, 1, 2,
				two_step_state);
			spr_copy_pattern_set(PATRN_SKELETON_CEILING, skeleton_ceiling, skeleton_ceiling_color);
			break;
		case PATRN_FISH:
			spr_define_pattern_set(PATRN_FISH, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_FISH, fish, fish_color);
			break;
		case PATRN_FIREBALL:
			spr_define_pattern_set(PATRN_FIREBALL, SPR_SIZE_16x16, 1, 1,
				bat_state);
			spr_copy_pattern_set(PATRN_FIREBALL, fireball, fireball_color);
			break;
		case PATRN_WATERDROP:
			spr_define_pattern_set(PATRN_WATERDROP, SPR_SIZE_16x16, 1, 1,
				waterdrop_state);
			spr_copy_pattern_set(PATRN_WATERDROP, waterdrop, waterdrop_color);
			break;
		case PATRN_BULLET:
			spr_define_pattern_set(PATRN_BULLET, SPR_SIZE_16x16, 1, 4,
				bullet_state);
			spr_copy_pattern_set(PATRN_BULLET, bullet, bullet_color);
			break;
	}

}
