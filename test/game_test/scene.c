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

#include "gen/game_test_tiles_ext.h"
#include "gen/game_test_sprites_ext.h"

#include "gen/map_defs.h"

#include "anim.h"
#include "logic.h"
#include "scene.h"

extern unsigned char *map_object_layer[25];
extern void sys_set_ascii_page3(char page);

struct tile_set tileset_map1;
struct tile_set tileset_map2;
struct tile_set tileset_map3;
struct tile_set tileset_map3b;
struct tile_set tileset_map4;
struct tile_set tileset_map5;
struct tile_set tileset_map6;
struct tile_set tileset_map7;


struct tile_set tileset[TILE_MAX];
struct tile_object tileobject[31];

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


uint8_t spr_ct, tob_ct;


void find_room_data(uint8_t room)
{
	map_object = (struct map_object_item *) map_object_layer[room];
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
	// before Allocating to VRAM tiles
	bool success;

	sys_set_ascii_page3(4);
	success = tile_set_valloc(&tileset[tileidx]);
	if (!success) {
		log_e("could not allocate tileobject\n");
		return;
	}
	sys_set_ascii_page3(7);
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
	sys_set_ascii_page3(7);
	spr_valloc_pattern_set(pattidx);
	spr_init_sprite(&enemy_sprites[objidx], pattidx);
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_set_pos(&enemy_sprites[objidx], map_object->x, map_object->y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	dpo->collision_state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_ct++;
}

extern unsigned char *map_map_segment_dict[25];
extern unsigned char *map_map_segment[25];

uint8_t *room_objs;

void load_room(uint8_t room)
{
	uint8_t i, id, type;
	bool add_dpo;
	spr_ct = 0, tob_ct = 0;

	vdp_screen_disable();
	tile_init();

	// fixme move elsewhere
	if (room == 8) {
		sys_set_ascii_page3(4);
		INIT_TILE_SET(tileset_map1, maptiles1);
		INIT_TILE_SET(tileset_map2, maptiles2);
		INIT_TILE_SET(tileset_map4, maptiles4);
		INIT_TILE_SET(tileset_map5, maptiles5);
		tile_set_valloc(&tileset_map1);
		tile_set_to_vram(&tileset_map4, 126);
		tile_set_to_vram(&tileset_map5, 126 + 32);
		sys_set_ascii_page3(6);
		INIT_TILE_SET(tileset_map3, maptiles3);
		tile_set_to_vram(&tileset_map3, 65);
		sys_set_ascii_page3(8);
		INIT_TILE_SET(tileset_map6, maptiles6);
		INIT_TILE_SET(tileset_map7, maptiles7);
		INIT_TILE_SET(tileset_map3b, maptiles3b);
		tile_set_to_vram(&tileset_map3b, 97);
		tile_set_to_vram(&tileset_map6, 126 + 64);
		tile_set_to_vram(&tileset_map7, 126 + 96);
	} else {
		sys_set_ascii_page3(4);
		INIT_TILE_SET(tileset_map1, maptiles1);
		INIT_TILE_SET(tileset_map2, maptiles2);
		INIT_TILE_SET(tileset_map4, maptiles4);
		INIT_TILE_SET(tileset_map5, maptiles5);
		tile_set_valloc(&tileset_map1);
		tile_set_valloc(&tileset_map2);
		tile_set_to_vram(&tileset_map4, 126);
		tile_set_to_vram(&tileset_map5, 126 + 32);
		sys_set_ascii_page3(6);
		INIT_TILE_SET(tileset_map3, maptiles3);
		tile_set_valloc(&tileset_map3);

	}
	sys_set_ascii_page3(6);

	// there is a mismatch between this two

	map_inflate(map_map_segment_dict[room], map_map_segment[room], scr_tile_buffer, 192, 32);

	spr_clear();
	phys_init();
	init_tile_collisions();

	// sys_set_rom();
	// spr_valloc_pattern_set(&spr_pattern[PATRN_MONK]);
	// spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	// sys_set_bios();

	INIT_LIST_HEAD(&display_list);

	// reading the map layer object requries swap4

	sys_set_ascii_page3(7);

	log_e("room : %d\n",room);
	//find_room_data(room);

	type = 0;
	room_objs = map_object_layer[room];
	for (dpo = display_object, i = 0; type != 255 ; i++, dpo++) {
		sys_set_ascii_page3(7);
		map_object = (struct map_object_item *) room_objs;
		type = map_object->type;
		log_e("type %d\n", type);
		log_e("room_objs %x\n", room_objs);
		if (type == ACTIONITEM) {
			uint8_t action_item_type = map_object->object.actionitem.type;
			log_e("action_item_type %d\n", action_item_type);
			if (action_item_type == TYPE_SCROLL) {
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
				add_tileobject(dpo, tob_ct, TILE_INVISIBLE_TRIGGER);
				phys_set_tile_collision_handler(dpo, trigger_handler, id);
			} else if (map_object->object.actionitem.type == TYPE_BELL) {
				if (!game_state.bell) {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					phys_set_tile_collision_handler(dpo, bell_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_BELL);
					dpo->tob->cur_anim_step = 1;
				}
			} else {
				room_objs = room_objs + sizeof(struct map_object_item)
							- sizeof(union map_object)
							+ sizeof(struct map_object_actionitem);
				continue;
			}
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_actionitem);
		} else if (map_object->type == STATIC) {
			if (map_object->object.static_.type == TYPE_DRAGON) {
 				// this is crashing, ignore
				//add_tileobject(dpo, tob_ct, TILE_DRAGON);
				// here there is some nice animation to do
			} else if (map_object->object.static_.type == TYPE_LAVA) {
				add_tileobject(dpo, tob_ct, TILE_LAVA);
				// also nice animation to do here
			} else if (map_object->object.static_.type == TYPE_SPEAR) {
				add_tileobject(dpo, tob_ct, TILE_SPEAR);
			} else if (map_object->object.static_.type == TYPE_WATER) {
				// this one also crashing?
				add_tileobject(dpo, tob_ct, TILE_WATER);
				//add_animator(dpo, ANIM_CYCLE_TILE);
			}
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_static);
		} else if (map_object->type == GHOST) {
			add_sprite(dpo, spr_ct, PATRN_GHOST);
			add_animator(dpo, ANIM_STATIC);
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_ghost);
		} else if (map_object->type == ROPE) {
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_rope);
		} else if (map_object->type == DOOR) {
			type = map_object->object.door.type;
			id = map_object->object.door.action_id;
			add_dpo = false;
			if (id == 0) {
				if (game_state.door_trigger)
					add_dpo = true;
			} else if (id == 1 && game_state.toggle[0] == 0) {
				add_dpo = true;
			} else if (id == 2 && !game_state.bell) {
				add_tileobject(dpo, tob_ct, TILE_TRAPDOOR);
				phys_set_colliding_tile_object(dpo, true);
			} else if (id == 3 && game_state.toggle[1] == 0) {
				add_dpo = true;
			} else if (id == 4 && game_state.toggle[2] == 0) {
				add_dpo = true;
			}
			if (add_dpo) {
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				if (type == 0) {
					dpo->tob->cur_anim_step = 1;
				}
				phys_set_colliding_tile_object(dpo, false);
			}
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_door);
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
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_shooter);
		} else if (map_object->type == BLOCK) {
			add_tileobject(dpo, tob_ct, TILE_CROSS);
			add_animator(dpo, ANIM_CYCLE_TILE);
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_block);
		} else if (map_object->type == STEP) {
			// add special collisions

			// need to see what to do with these ones
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_step);
		} else if (map_object->type == MOVABLE) {
			if (map_object->object.movable.type == TYPE_TEMPLAR) {
				add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_BAT) {
				add_sprite(dpo, spr_ct, PATRN_BAT);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_SPIDER) {
				add_sprite(dpo, spr_ct, PATRN_SPIDER);
				add_animator(dpo, ANIM_UP_DOWN);
				 dpo->state = STATE_MOVING_DOWN;
			} else if (map_object->object.movable.type == TYPE_RAT) {
				add_sprite(dpo, spr_ct, PATRN_RAT);
				add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.movable.type == TYPE_WORM) {
				add_sprite(dpo, spr_ct, PATRN_WORM);
				add_animator(dpo, ANIM_LEFT_RIGHT);
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
				add_animator(dpo, ANIM_LEFT_RIGHT);
			// } else if (map_object->object.movable.type == TYPE_DEATH) {
			// 	// this is a big sprite 32x32 not supported yet
			// 	map_object++;
			// 	continue;
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
				room_objs = room_objs + sizeof(struct map_object_item)
							- sizeof(union map_object)
							+ sizeof(struct map_object_movable);
				continue;
			}
			room_objs = room_objs + sizeof(struct map_object_item)
						- sizeof(union map_object)
						+ sizeof(struct map_object_movable);
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
			log_e("showing sprite\n");
			spr_show(dpo->spr);
		} else if (dpo->type == DISP_OBJECT_TILE) {
			log_e("showing dpo\n");
			tile_object_show(dpo->tob, scr_tile_buffer, false);
		}
	}


	vdp_copy_to_vram(scr_tile_buffer, vdp_base_names_grp1, 704);

	vdp_screen_enable();
}



// void init_monk()sys_set_ascii_page3(4);  // PAGE 1 -- it would be good to define this with labels
// {
// 	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
// 	dpo_monk.xpos = 100;
// 	dpo_monk.ypos = 192 - 64;
// 	dpo_monk.vy = 0;
// 	dpo_monk.type = DISP_OBJECT_SPRITE;
// 	dpo_monk.state = STATE_ONGROUND;
// 	dpo_monk.spr = &monk_sprite;
// 	dpo_monk.collision_state = 0;
// 	spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
// }

void init_tile_collisions()
{
	uint8_t i;
	for (i = 1; i < 86; i++)
		phys_set_colliding_tile(i);

	phys_clear_colliding_tile(16); // step brown
	phys_clear_colliding_tile(38); // step white
	phys_set_down_colliding_tile(16);
	phys_set_down_colliding_tile(38);
}

void init_resources()
{
	uint8_t two_step_state[] = {2,2};
	uint8_t single_step_state[] = {1,1};
	uint8_t three_step_state[] = {3,3};
	uint8_t bullet_state[] = {1,1};
	uint8_t bat_state[] = {2};
	uint8_t waterdrop_state[] = {3};
	uint8_t single_four_state[] = {4};
	uint8_t spider_state[]={2};
	uint8_t archer_state[]={2,2};


	tile_init();
	//tile_set_to_vram(&tileset_map3b, 90);

	/** load font tileset */
	// INIT_TILE_SET(tileset[TILE_FONT_DIGITS], font_digits);
	// INIT_TILE_SET(tileset[TILE_FONT_UPPER], font_upper);
	// INIT_TILE_SET(tileset[TILE_FONT_LOWER], font_lower);
	// tile_set_valloc(&tileset[TILE_FONT_DIGITS]);
	// tile_set_valloc(&tileset[TILE_FONT_UPPER]);
	// tile_set_valloc(&tileset[TILE_FONT_LOWER]);

	sys_set_ascii_page3(4);
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
	INIT_DYNAMIC_TILE_SET(tileset[TILE_DOOR], door, 1, 4, 2, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_TRAPDOOR], trapdoor, 2, 2, 1, 1);
	INIT_DYNAMIC_TILE_SET(tileset[TILE_INVISIBLE_TRIGGER], invisible_trigger, 1, 4, 1, 1);

	sys_set_ascii_page3(7);
	spr_init();
	/** initialize sprite pattern sets */
	SPR_DEFINE_PATTERN_SET(PATRN_BAT, SPR_SIZE_16x16, 1, 1, bat_state, bat);
	SPR_DEFINE_PATTERN_SET(PATRN_RAT, SPR_SIZE_16x16, 1, 2, two_step_state, rat);
	SPR_DEFINE_PATTERN_SET(PATRN_SPIDER, SPR_SIZE_16x16, 1, 1, bat_state, spider);
	SPR_DEFINE_PATTERN_SET(PATRN_MONK, SPR_SIZE_16x32, 1, 2, three_step_state, monk1);
	SPR_DEFINE_PATTERN_SET(PATRN_TEMPLAR, SPR_SIZE_16x32, 1, 2, two_step_state, templar);
	SPR_DEFINE_PATTERN_SET(PATRN_WORM, SPR_SIZE_16x16, 1, 2, two_step_state, worm);
	SPR_DEFINE_PATTERN_SET(PATRN_SKELETON, SPR_SIZE_16x32, 1, 2, two_step_state, skeleton);
	SPR_DEFINE_PATTERN_SET(PATRN_PALADIN, SPR_SIZE_16x32, 1, 2, two_step_state, paladin);
	SPR_DEFINE_PATTERN_SET(PATRN_GUADANYA, SPR_SIZE_16x16, 1, 1, single_four_state, guadanya);
	SPR_DEFINE_PATTERN_SET(PATRN_GHOST, SPR_SIZE_16x16, 1, 2, two_step_state, ghost);
	SPR_DEFINE_PATTERN_SET(PATRN_DEMON, SPR_SIZE_16x32, 1, 2, two_step_state, demon);
//	SPR_DEFINE_PATTERN_SET(PATRN_DEATH, SPR_SIZE_32x32, 1, 2, 2, death);
	SPR_DEFINE_PATTERN_SET(PATRN_DARKBAT, SPR_SIZE_16x16, 1, 2, two_step_state, darkbat);
	SPR_DEFINE_PATTERN_SET(PATRN_FLY, SPR_SIZE_16x16, 1, 2, two_step_state, fly);
	SPR_DEFINE_PATTERN_SET(PATRN_SKELETON_CEILING, SPR_SIZE_16x32, 1, 2, two_step_state, skeleton_ceiling);
	SPR_DEFINE_PATTERN_SET(PATRN_FISH, SPR_SIZE_16x16, 1, 1, bat_state, fish);
	SPR_DEFINE_PATTERN_SET(PATRN_FIREBALL, SPR_SIZE_16x16, 1, 1, bat_state, fireball);
	SPR_DEFINE_PATTERN_SET(PATRN_WATERDROP, SPR_SIZE_16x16, 1, 1, waterdrop_state, waterdrop);

        //init_monk();
}
