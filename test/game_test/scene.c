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

/** room title tilesets **/
struct tile_set tileset_room_title[ROOM_MAX];

/** object tilesets **/
struct tile_set tileset[TILE_MAX];

/** scene primitives **/
struct tile_object tileobject[SCENE_MAX_DPO];
struct tile_object bullet_tob[SCENE_MAX_TOB_BULLET];
struct spr_sprite_def enemy_sprites[SCENE_MAX_DPO];
struct spr_sprite_def bullet_sprites[SCENE_MAX_BULLET];
struct spr_sprite_def jean_sprite;

// FIXME: there appears to be a buffer overrun in one of this structures,
//        because rearranging them in the wrong order leads to a hang on start

/** scene display objects **/
struct displ_object display_object[SCENE_MAX_DPO];
struct displ_object dpo_bullet[SCENE_MAX_BULLET];
struct displ_object dpo_tob_bullet[SCENE_MAX_TOB_BULLET];

/** main character display object **/
struct displ_object dpo_jean;

/** temporary dpo **/
struct displ_object *tmp_dpo;

/** main display list **/
struct list_head display_list;

/** current map object **/
struct map_object_item *map_object;

/** iterators to check for sprite collisions **/
struct list_head *coll_elem;
struct displ_object *coll_dpo;

/** tile object and sprite counters **/
uint8_t spr_ct, tob_ct, bullet_ct;

/** current room object data **/
uint8_t *room_objs;

extern void init_room_tilesets(uint8_t room, bool reload);
extern void init_sfx();
extern void show_room_title(uint8_t room);
extern void start_music(uint8_t room);

static void add_tileobject(struct displ_object *dpo, uint8_t objidx,
	enum tile_sets_t tileidx)
{
	if (tileset[tileidx].raw)
		ascii8_set_data(PAGE_RAW_TILES);
	else
		ascii8_set_data(PAGE_DYNTILES);
	tile_set_valloc(&tileset[tileidx]);

	ascii8_set_data(PAGE_MAPOBJECTS);
	tileobject[objidx].x = map_object->x;
	tileobject[objidx].y = map_object->y;
	tileobject[objidx].cur_dir = 0;
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
	ascii8_set_data(PAGE_SPRITES);
	spr_valloc_pattern_set(pattidx);
	spr_init_sprite(&enemy_sprites[objidx], pattidx);

	ascii8_set_data(PAGE_MAPOBJECTS);
	spr_set_pos(&enemy_sprites[objidx], map_object->x, map_object->y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = map_object->x;
	dpo->ypos = map_object->y;
	dpo->state = 0;
	dpo->visible = true;
	dpo->collision_state = 0;
	dpo->check_collision = true;

	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
	spr_ct++;
}

void add_jean() __nonbanked
{
	ascii8_set_data(PAGE_SPRITES);
	spr_valloc_pattern_set(PATRN_JEAN);

	spr_init_sprite(&jean_sprite, PATRN_JEAN);
	jean_sprite.cur_state = game_state.jean_anim_state;

	dpo_jean.xpos = game_state.jean_x;
	dpo_jean.ypos = game_state.jean_y;
	dpo_jean.type = DISP_OBJECT_SPRITE;
	dpo_jean.state = game_state.jean_state;
	dpo_jean.spr = &jean_sprite;
	dpo_jean.visible = true;
	dpo_jean.collision_state = game_state.jean_collision_state;
	dpo_jean.check_collision = false;
	spr_set_pos(&jean_sprite, dpo_jean.xpos, dpo_jean.ypos);
	INIT_LIST_HEAD(&dpo_jean.animator_list);

	if (game_state.room == ROOM_BONFIRE) {
		dpo_jean.xpos = 124;
		dpo_jean.ypos = 100;
		spr_set_pos(&jean_sprite, dpo_jean.xpos, dpo_jean.ypos);
		add_animator(&dpo_jean, ANIM_JEAN_BONFIRE);
	} else if (game_state.room == ROOM_EVIL_CHAMBER) {
		dpo_jean.xpos = 200;
		dpo_jean.ypos = 96;
		jean_sprite.cur_state = SPR_STATE_LEFT;
		spr_set_pos(&jean_sprite, dpo_jean.xpos, dpo_jean.ypos);
	} else {
		add_animator(&dpo_jean, ANIM_JEAN);
	}
}

/**
 * Add bullet generic
 */
void add_bullet(uint8_t xpos, uint8_t ypos, uint8_t patrn_id, uint8_t anim_id,
	uint8_t state, uint8_t dir, uint8_t speed, struct displ_object *parent)
{
	uint8_t idx;

	idx = 0;
	for (idx = 0; idx < SCENE_MAX_BULLET; idx++) {
		if (dpo_bullet[idx].state == 255)
			break;
	}

	if (idx == SCENE_MAX_BULLET)
		return;

	spr_init_sprite(&bullet_sprites[idx], patrn_id);
	bullet_sprites[idx].cur_state = state;
	bullet_sprites[idx].cur_anim_step = 0;
	spr_set_pos(&bullet_sprites[idx], xpos, ypos);

	dpo_bullet[idx].type = DISP_OBJECT_SPRITE;
	dpo_bullet[idx].spr = &bullet_sprites[idx];
	dpo_bullet[idx].xpos = xpos;
	dpo_bullet[idx].ypos = ypos;
	dpo_bullet[idx].state = state;
	dpo_bullet[idx].collision_state = 0;
	dpo_bullet[idx].aux = dir;
	dpo_bullet[idx].aux2 = speed;
	dpo_bullet[idx].parent = parent;
	dpo_bullet[idx].visible = true;
	dpo_bullet[idx].check_collision = true;

	INIT_LIST_HEAD(&dpo_bullet[idx].list);
	INIT_LIST_HEAD(&dpo_bullet[idx].animator_list);
	add_animator(&dpo_bullet[idx], anim_id);
	list_add(&dpo_bullet[idx].list, &display_list);
	spr_show(dpo_bullet[idx].spr);
}

/**
 * Add dragon flame
 */
void add_tob_bullet(uint8_t xpos, uint8_t ypos, uint8_t tileidx, uint8_t anim_id,
	uint8_t state, uint8_t dir, uint8_t speed, struct displ_object *parent)
{
	uint8_t idx;

	ascii8_set_data(PAGE_DYNTILES);
	tile_set_valloc(&tileset[tileidx]);

	idx = 0;
	for (idx = 0; idx < SCENE_MAX_TOB_BULLET; idx++) {
		if (dpo_tob_bullet[idx].state == 255)
			break;
	}

	if (idx == SCENE_MAX_TOB_BULLET)
		return;

	bullet_tob[idx].x = xpos;
	bullet_tob[idx].y = ypos;
	bullet_tob[idx].cur_dir = 0;
	bullet_tob[idx].cur_anim_step = 0;
	bullet_tob[idx].ts = &tileset[tileidx];
	bullet_tob[idx].idx = 0;

	dpo_tob_bullet[idx].type = DISP_OBJECT_TILE;
	dpo_tob_bullet[idx].tob = &bullet_tob[idx];
	dpo_tob_bullet[idx].xpos = xpos;
	dpo_tob_bullet[idx].ypos = ypos;
	dpo_tob_bullet[idx].visible = true;
	dpo_tob_bullet[idx].state = state;
	dpo_tob_bullet[idx].aux = dir;
	dpo_tob_bullet[idx].aux2 = speed;
	dpo_tob_bullet[idx].parent = parent;
	INIT_LIST_HEAD(&dpo_tob_bullet[idx].list);
	list_add(&dpo_tob_bullet[idx].list, &display_list);
	INIT_LIST_HEAD(&dpo_tob_bullet[idx].animator_list);
	add_animator(&dpo_tob_bullet[idx], anim_id);
	tile_object_show(dpo_tob_bullet[idx].tob, scr_tile_buffer, true);
	phys_set_colliding_tile_object(&dpo_tob_bullet[idx],
		TILE_COLLISION_TRIGGER, deadly_tile_handler, 0);
}

inline bool jean_check_collision() __nonbanked
{
	uint8_t spr_size = coll_dpo->spr->pattern_set->size;
	int16_t box_x, box_y, box_w, box_h;;

	/** Hack to optimize final boss bullet performance **/
	if (game_state.room == ROOM_SATAN) {
		return true;
	}
	switch(spr_size) {
	 	case SPR_SIZE_16x16:
	 		box_x = 16; box_w = 16; box_y = 32; box_h = 16;
			break;
	 	case SPR_SIZE_16x32:
			/** less strict to allow jumping over enemies */
			box_x = 10; box_w = 10; box_y = 16; box_h = 32;
	 		break;
	 	case SPR_SIZE_32x16:
			box_x = 10; box_w = 32; box_y = 32; box_h = 16;
			break;
	 	case SPR_SIZE_32x32:
			box_x = 16; box_w = 32; box_y = 32; box_h = 32;
			break;
	}
	if (coll_dpo->xpos < (dpo_jean.xpos + box_x) &&
		(coll_dpo->xpos + box_w) > dpo_jean.xpos) {
		if (dpo_jean.state == STATE_CROUCHING) {
			if (coll_dpo->ypos < (dpo_jean.ypos + box_y) &&
				(coll_dpo->ypos + box_h - 8) > (dpo_jean.ypos + 16)) {
				return true;
			}
		} else {
			if (coll_dpo->ypos < (dpo_jean.ypos + box_y) &&
				(coll_dpo->ypos + box_h) > dpo_jean.ypos + 8) {
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
			if (coll_dpo->type == DISP_OBJECT_SPRITE && coll_dpo->check_collision)
				if (jean_check_collision()) {
					 dpo_jean.state = STATE_COLLISION;
					 return;
				}
		}
	}
}

extern uint8_t *current_song;

void init_scene()
{
	spr_ct = 0;
	tob_ct = 0;
	bullet_ct = 0;
	current_song = NULL;
}

void clear_room()
{
	uint8_t i;

	/* clear all sprite attributes and patterns from VRAM but
	   _not_ sprite definitions in RAM */
	spr_clear();

	phys_clear_sprite_collision_handler();

	/* free dynamic tiles, but do not clear patterns from VRAM */
	for (i = 0; i < tob_ct; i++) {
		tile_set_vfree(tileobject[i].ts);
	}

	/* free dragon bullets */
	for (i = 0; i < SCENE_MAX_TOB_BULLET; i++) {
		tile_set_vfree(bullet_tob[i].ts);
	}

	spr_ct = 0;
	tob_ct = 0;
	bullet_ct = 0;

	for (i = 0; i < SCENE_MAX_BULLET; i++) {
		dpo_bullet[i].state = 255;
	}
	for (i = 0; i < SCENE_MAX_TOB_BULLET; i++) {
		dpo_tob_bullet[i].state = 255;
	}
}

void load_intro_scene() __nonbanked
{
	uint8_t i;
	struct map_object_item tmp_obj;
	map_object = &tmp_obj;

	spr_clear();

	INIT_LIST_HEAD(&display_list);

	for (i = 0; i < 3; i++) {
		dpo = &display_object[i];
		tmp_obj.x = 0;
		tmp_obj.y = 80;
		add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
		add_animator(dpo, ANIM_INTRO_CHASE);
		dpo->state = STATE_OFF_SCREEN;
		dpo->visible = false;
		dpo->aux = 0;
		dpo->spr->cur_state = SPR_STATE_RIGHT;
		if (i == 1) {
			dpo->state = STATE_OFF_SCREEN_DELAY_1S;
		} else if (i == 2) {
			dpo->state = STATE_OFF_SCREEN_DELAY_2S;
		}
	}

	ascii8_set_data(PAGE_SPRITES);
	spr_valloc_pattern_set(PATRN_JEAN);
	spr_init_sprite(&jean_sprite, PATRN_JEAN);

	jean_sprite.cur_state = JANE_STATE_RIGHT;
	dpo_jean.xpos = 20;
	dpo_jean.ypos = 80;
	dpo_jean.type = DISP_OBJECT_SPRITE;
	dpo_jean.state = STATE_MOVING_RIGHT;
	dpo_jean.spr = &jean_sprite;
	dpo_jean.collision_state = 0;
	dpo_jean.check_collision = false;
	dpo_jean.visible = true;
	spr_set_pos(&jean_sprite, dpo_jean.xpos, dpo_jean.ypos);
	INIT_LIST_HEAD(&dpo_jean.list);
	list_add(&dpo_jean.list, &display_list);
	INIT_LIST_HEAD(&dpo_jean.animator_list);
	add_animator(&dpo_jean, ANIM_INTRO_JEAN);

	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
			spr_show(dpo->spr);
		}
	}
}


void load_room(uint8_t room, bool reload)
{
	uint8_t i, id, type, delay, offset, damage;
	bool add_dpo;
	struct spr_pattern_set *ps;

	clear_room();
	game_state.templar_ct = 0;
	vdp_screen_disable();

	init_room_tilesets(room, reload);


	ascii8_set_data(PAGE_MAP);
	map_inflate(map_map_segment_dict[room], map_map_segment[room], scr_tile_buffer, 192, 32);

	init_sfx();

	INIT_LIST_HEAD(&display_list);

	ascii8_set_data(PAGE_MAPOBJECTS);

	type = 0;
	room_objs = map_object_layer[room];

	for (dpo = display_object, i = 0; type != 255 ; i++, dpo++) {
		ascii8_set_data(PAGE_MAPOBJECTS);
		map_object = (struct map_object_item *) room_objs;
		type = map_object->type;
		if (type == ACTIONITEM) {
			uint8_t action_item_type = map_object->object.actionitem.type;
			if (action_item_type == TYPE_SCROLL) {
				id = map_object->object.actionitem.action_id;
				if (id < 7) {
					if (game_state.scroll[id] == 0) {
						add_tileobject(dpo, tob_ct, TILE_SCROLL);
						phys_set_colliding_tile_object(dpo,
							TILE_COLLISION_TRIGGER, pickup_scroll, id);
					}
				} else {
					add_tileobject(dpo, tob_ct, TILE_RED_SCROLL);
					dpo->visible = false;
					add_animator(dpo, ANIM_RED_PARCHMENT);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, pickup_red_scroll, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_TOGGLE) {
				id = map_object->object.actionitem.action_id;
				if (game_state.toggle[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, toggle_handler, id);
				} else {
					add_tileobject(dpo, tob_ct, TILE_TOGGLE);
				}
			} else if (map_object->object.actionitem.type == TYPE_CROSS) {
				/**
				 * This is a complicated corner case: two objects need to
				 * be switched over when an external action happens, and
				 * also we can have multiple copies on the room with different
				 * states each. Finally, it is possible to pick up each one
				 * of the items when in one of the states...
				 */
				id = map_object->object.actionitem.action_id - 1;
				if (id < 8 && game_state.cross[id] == 0) {
					if (!game_state.cross_switch) {
						add_tileobject(dpo, tob_ct, TILE_INVERTED_CROSS);
						dpo->aux = 1;
						add_animator(dpo, ANIM_CROSS);
						dpo->visible = false;
						tmp_dpo = dpo;
						dpo++;
						add_tileobject(dpo, tob_ct, TILE_CROSS);
						dpo->aux = 0;
						dpo->parent = tmp_dpo;
						add_animator(dpo, ANIM_CROSS);
						phys_set_colliding_tile_object(dpo,
							TILE_COLLISION_TRIGGER
							| TILE_COLLISION_MULTIPLE, pickup_cross, id);
					} else {
						add_tileobject(dpo, tob_ct, TILE_INVERTED_CROSS);
						dpo->aux = 1;
						add_animator(dpo, ANIM_CROSS);
						tmp_dpo = dpo;
						dpo++;
						add_tileobject(dpo, tob_ct, TILE_CROSS);
						dpo-> aux = 0;
						dpo-> parent = tmp_dpo;
						add_animator(dpo, ANIM_CROSS);
						phys_set_colliding_tile_object(dpo,
							TILE_COLLISION_TRIGGER
							| TILE_COLLISION_MULTIPLE, pickup_cross, id);
						dpo->visible = false;
					}
				} else if (id > 7 && game_state.cross[id] == 0) {
					if (!game_state.cross_switch) {
						add_tileobject(dpo, tob_ct, TILE_INVERTED_CROSS);
						dpo->aux = 0;
						add_animator(dpo, ANIM_CROSS);
						tmp_dpo = dpo;
						dpo++;
						add_tileobject(dpo, tob_ct, TILE_CROSS);
						dpo->aux = 1;
						dpo->parent = tmp_dpo;
						add_animator(dpo, ANIM_CROSS);
						phys_set_colliding_tile_object(dpo,
							TILE_COLLISION_TRIGGER
							|TILE_COLLISION_MULTIPLE, pickup_cross, id);
						dpo->visible = false;
					} else {
						add_tileobject(dpo, tob_ct, TILE_INVERTED_CROSS);
						dpo->aux = 0;
						add_animator(dpo, ANIM_CROSS);
						dpo->visible = false;
						tmp_dpo = dpo;
						dpo++;
						add_tileobject(dpo, tob_ct, TILE_CROSS);
						dpo->aux = 1;
						dpo->parent = tmp_dpo;
						add_animator(dpo, ANIM_CROSS);
						phys_set_colliding_tile_object(dpo,
							TILE_COLLISION_TRIGGER
							|TILE_COLLISION_MULTIPLE, pickup_cross, id);
					}
				}
			} else if (map_object->object.actionitem.type == TYPE_TELETRANSPORT) {
				id = map_object->object.actionitem.action_id;
				add_tileobject(dpo, tob_ct, TILE_TELETRANSPORT);
				/** portal is only enabled if we enter the room from a doorway */
				if (game_state.teletransport) {
					game_state.teletransport = false;
				} else {
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, teletransport, id);
				}
			} else if (map_object->object.actionitem.type == TYPE_HEART) {
				id = map_object->object.actionitem.action_id;
				if (game_state.hearth[id] == 0) {
					add_tileobject(dpo, tob_ct, TILE_HEART);
					add_animator(dpo, ANIM_CYCLE_TILE);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER
						|TILE_COLLISION_MULTIPLE, pickup_heart, id);
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
				add_sprite(dpo, spr_ct, PATRN_SWITCH_MASK);
				dpo->check_collision = false;
				ps = &spr_pattern[PATRN_SWITCH_MASK];
				ps->colors2[0] = 1;
				ps->colors2[1] = 1;
				tmp_dpo = dpo;
				dpo++;
				add_tileobject(dpo, tob_ct, TILE_SWITCH);
				dpo->parent = tmp_dpo;
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_TRIGGER, crosswitch_handler, 0);
				if(game_state.cross_switch) {
					dpo->tob->cur_anim_step = 1;
					dpo->parent->spr->cur_anim_step = 1;
				}
			} else if (map_object->object.actionitem.type == TYPE_CUP) {
				// Can only fight final boss if we have all 12 crosses
				if (game_state.cross_cnt == 12
					|| game_state.final_fight) {
					game_state.final_fight = true;
					add_tileobject(dpo, tob_ct, TILE_CUP);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, cup_handler, 0);
				}
			} else if (map_object->object.actionitem.type == TYPE_TRIGGER) {
				id = map_object->object.actionitem.action_id;
				add_tileobject(dpo, tob_ct, TILE_INVISIBLE_TRIGGER);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_TRIGGER, trigger_handler, id);
			} else if (map_object->object.actionitem.type == TYPE_BELL) {
				if (!game_state.bell) {
					add_sprite(dpo, spr_ct, PATRN_BELL_MASK);
					dpo->check_collision = false;
					ps = &spr_pattern[PATRN_BELL_MASK];
					ps->colors2[0] = 1;
					ps->colors2[1] = 1;
					tmp_dpo = dpo;
					dpo++;
					add_tileobject(dpo, tob_ct, TILE_BELL);
					dpo->parent = tmp_dpo;
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, bell_handler, id);
				} else {
					add_sprite(dpo, spr_ct, PATRN_BELL_MASK);
					ps = &spr_pattern[PATRN_BELL_MASK];
					ps->colors2[0] = 1;
					ps->colors2[1] = 1;
					dpo->spr->cur_anim_step = 1;
					dpo->check_collision = false;
					dpo++;
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
				add_tileobject(dpo, tob_ct, TILE_DRAGON);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
			} else if (map_object->object.static_.type == TYPE_LAVA) {
				offset = map_object->object.static_.offset;
				add_tileobject(dpo, tob_ct, TILE_LAVA);
				dpo->tob->cur_anim_step = offset;
				add_animator(dpo, ANIM_LAVA);
				if (room != ROOM_BONFIRE) {
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
				}
			} else if (map_object->object.static_.type == TYPE_LAVA4) {
				add_tileobject(dpo, tob_ct, TILE_LAVA4);
				add_animator(dpo, ANIM_LAVA);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
			} else if (map_object->object.static_.type == TYPE_LAVA6) {
				add_tileobject(dpo, tob_ct, TILE_LAVA6);
				add_animator(dpo, ANIM_LAVA);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
			} else if (map_object->object.static_.type == TYPE_SPEAR) {
				damage = map_object->object.static_.damage;
				if (damage == 1) {
					add_tileobject(dpo, tob_ct, TILE_HARMLESS_SPEAR);
				} else {
					add_tileobject(dpo, tob_ct, TILE_SPEAR);
					phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
				}
			} else if (map_object->object.static_.type == TYPE_WATER) {
				add_tileobject(dpo, tob_ct, TILE_WATER);
				add_animator(dpo, ANIM_WATER);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
			}
			room_objs += NEXT_OBJECT(struct map_object_static);
		} else if (map_object->type == GHOST) {
			dpo->speed = map_object->object.ghost.speed;
			add_sprite(dpo, spr_ct, PATRN_GHOST);
			dpo->aux = 0;
			add_animator(dpo, ANIM_GHOST);
			room_objs += NEXT_OBJECT(struct map_object_ghost);
		} else if (map_object->type == ROPE) {
			room_objs += NEXT_OBJECT(struct map_object_rope);
		} else if (map_object->type == DOOR) {
			type = map_object->object.door.type;
			id = map_object->object.door.action_id;
			add_dpo = false;
			if (id == 0) {
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				dpo->visible = game_state.door_trigger;
				add_animator(dpo, ANIM_CLOSE_DOOR);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			} else if (id == 1 && game_state.toggle[0] == 0) {
				/** hanging priests, opens with dragon toggle **/
				add_dpo = true;
			} else if (id == 2 && !game_state.bell) {
				add_tileobject(dpo, tob_ct, TILE_TRAPDOOR);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			} else if (id == 3 && game_state.toggle[1] == 0) {
				/** death door, opens with toggle in hangin priests */
				add_dpo = true;
			} else if (id == 4) {
				/** evil church door, open with toggle room */
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				dpo->tob->cur_dir = 1;
				dpo->tob->cur_anim_step = 0;
				dpo->visible = game_state.toggle[2] == 0;
				add_animator(dpo, ANIM_OPEN_DOOR);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			} else if (id == 5) {
				// church tower door
				add_dpo = true;
			} else if (id == 6
				&& (game_state.cross_cnt == 12
				|| game_state.final_fight)) {
				/* satan door */
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				dpo->visible = false;
				add_animator(dpo, ANIM_CLOSE_DOOR_SATAN);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			}
			if (add_dpo) {
				add_tileobject(dpo, tob_ct, TILE_DOOR);
				if (type == 1) {
					dpo->tob->cur_dir = 1;
					dpo->tob->cur_anim_step = 0;
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
				dpo->aux2 = 0;
				add_animator(dpo, ANIM_SPLASH);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_FISH);
			} else if (map_object->object.shooter.type == TYPE_LEAK) {
				dpo->max = map_object->object.shooter.max;
				add_sprite(dpo, spr_ct, PATRN_WATERDROP);
				add_animator(dpo, ANIM_WATERDROP);
			} else if (map_object->object.shooter.type == TYPE_GARGOYLE) {
				add_tileobject(dpo, tob_ct, TILE_GARGOLYNE);
				dpo->aux = 30;
				add_animator(dpo, ANIM_GARGOLYNE);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_SPIT);
			} else if (map_object->object.shooter.type == TYPE_ARCHER) {
				add_tileobject(dpo, tob_ct, TILE_ARCHER_SKELETON);
				dpo->aux = 30;
				add_animator(dpo, ANIM_ARCHER_SKELETON);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, deadly_tile_handler, 0);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_ARROW);
			} else if (map_object->object.shooter.type == TYPE_PLANT) {
				delay = map_object->object.shooter.delay;
				dpo->aux = delay;
				add_tileobject(dpo, tob_ct, TILE_PLANT);
				add_animator(dpo, ANIM_SHOOTER_PLANT);
				phys_set_masked_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, 0, 1, 2, 1,
						deadly_tile_handler, 0);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_SMALL_BULLET);
			} else if (map_object->object.shooter.type == TYPE_FLAME) {
				add_tileobject(dpo, tob_ct, TILE_FLAME);
				add_animator(dpo, ANIM_DRAGON_FLAME);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_TRIGGER, deadly_tile_handler, 0);
			}
			room_objs += NEXT_OBJECT(struct map_object_shooter);
		} else if (map_object->type == BLOCK) {
			// Can only fight boss if we have all 12 crosses
			if (game_state.cross_cnt == 12
				|| game_state.final_fight) {
				game_state.final_fight = true;
				id = map_object->object.block.index;
				add_tileobject(dpo, tob_ct, TILE_BLOCK_CROSS);
				dpo->tob->cur_anim_step = id % 3;
				dpo->aux = id;
				dpo->state = 0;
				dpo->visible = false;
				add_animator(dpo, ANIM_BLOCK_CROSSES);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_FULL, null_handler, 0);
			}
			room_objs += NEXT_OBJECT(struct map_object_block);
		} else if (map_object->type == STEP) {
			// TODO: Add special collisions
			room_objs += NEXT_OBJECT(struct map_object_step);
		} else if (map_object->type == STAINEDGLASS) {
			add_tileobject(dpo, tob_ct, TILE_STAINED_GLASS);
			room_objs += NEXT_OBJECT(struct map_object_stainedglass);
		} else if (map_object->type == PENTAGRAM) {
			add_tileobject(dpo, tob_ct, TILE_PENTAGRAM);
			room_objs += NEXT_OBJECT(struct map_object_stainedglass);
		} else if (map_object->type == MOVABLE) {
			dpo->speed = map_object->object.movable.speed;
			dpo->min = map_object->object.movable.min;
			dpo->max = map_object->object.movable.max;
			dpo->state = map_object->object.movable.dir;
			if (map_object->object.movable.type == TYPE_TEMPLAR) {
				add_sprite(dpo, spr_ct, PATRN_TEMPLAR);
				if (room == ROOM_FOREST ||
					room == ROOM_GRAVEYARD) {
					if (room == ROOM_FOREST)
						add_animator(dpo, ANIM_CHASE_FOREST);
					else
						add_animator(dpo, ANIM_CHASE_GRAVEYARD);
					dpo->state = STATE_OFF_SCREEN;
					dpo->visible = false;
					dpo->xpos = -32;
					dpo->aux = 0;
					dpo->aux2 = 0;
					dpo->spr->cur_state = SPR_STATE_RIGHT;
					if (game_state.templar_ct == 1) {
						dpo->xpos = -56;
					} else if (game_state.templar_ct == 2) {
						dpo->xpos = -80;
					}
					game_state.templar_ct++;
				} else if (room == ROOM_EVIL_CHAMBER) {
					add_animator(dpo, ANIM_EVIL_CHAMBER);
					dpo->state = STATE_OFF_SCREEN;
					dpo->visible = false;
					dpo->aux = 0;
					dpo->aux2 = 0;
					dpo->spr->cur_state = SPR_STATE_RIGHT;
					if (game_state.templar_ct == 1) {
						dpo->state = STATE_OFF_SCREEN_DELAY_1S;
					} else if (game_state.templar_ct == 2) {
						dpo->state = STATE_OFF_SCREEN_DELAY_2S;
					} else if (game_state.templar_ct == 3) {
						dpo->state = STATE_OFF_SCREEN_DELAY_3S;
					}
					game_state.templar_ct++;
				} else if (room == ROOM_BONFIRE) {
					add_animator(dpo, ANIM_TEMPLAR_BONFIRE);
					dpo->spr->cur_state = SPR_STATE_RIGHT;
					if (game_state.templar_ct > 1) {
						dpo->spr->cur_state = SPR_STATE_LEFT;
					}
					game_state.templar_ct++;
				}
			} else if (map_object->object.movable.type == TYPE_BAT) {
				add_sprite(dpo, spr_ct, PATRN_BAT);
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_SPIDER) {
				add_sprite(dpo, spr_ct, PATRN_SPIDER);
				add_animator(dpo, ANIM_UP_DOWN_BOUNDED);
				dpo->state = STATE_MOVING_DOWN;
			} else if (map_object->object.movable.type == TYPE_RAT) {
				add_sprite(dpo, spr_ct, PATRN_RAT);
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_WORM) {
				add_sprite(dpo, spr_ct, PATRN_WORM);
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_PRIEST) {
				add_tileobject(dpo, tob_ct, TILE_PRIEST);
				dpo->state = STATE_MOVING_DOWN;
				add_animator(dpo, ANIM_HANGING_PRIEST);
				phys_set_colliding_tile_object(dpo,
					TILE_COLLISION_TRIGGER, deadly_tile_handler, 0);
			} else if (map_object->object.movable.type == TYPE_FLY) {
				add_sprite(dpo, spr_ct, PATRN_FLY);
				add_animator(dpo, ANIM_UP_DOWN_BOUNDED);
				dpo->state = STATE_MOVING_DOWN;
			} else if (map_object->object.movable.type == TYPE_SKELETON) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON);
				dpo->state = STATE_MOVING_LEFT;
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_PALADIN) {
				add_sprite(dpo, spr_ct, PATRN_PALADIN);
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_DEATH) {
				add_sprite(dpo, spr_ct, PATRN_DEATH);
				dpo->aux = 3; // max scythes
				dpo->aux2 = 0; // scythe count
				dpo->state = STATE_MOVING_RIGHT;
				add_animator(dpo, ANIM_DEATH);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_SCYTHE);
			} else if (map_object->object.movable.type == TYPE_DARK_BAT) {
				add_sprite(dpo, spr_ct, PATRN_DARKBAT);
				dpo->state = STATE_MOVING_LEFT;
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_DEMON) {
				add_sprite(dpo, spr_ct, PATRN_DEMON);
				dpo->state = STATE_MOVING_LEFT;
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_SKELETON_CEIL) {
				add_sprite(dpo, spr_ct, PATRN_SKELETON_CEILING);
				dpo->state = STATE_MOVING_LEFT;
				add_animator(dpo, ANIM_LEFT_RIGHT_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_LAVA) {
				add_sprite(dpo, spr_ct, PATRN_FIREBALL);
				dpo->state = STATE_MOVING_UP;
				add_animator(dpo, ANIM_UP_DOWN_BOUNDED);
			} else if (map_object->object.movable.type == TYPE_SATAN) {
				add_tileobject(dpo, tob_ct, TILE_SATAN);
				dpo->aux = 0;
				dpo->aux2 = 50;
				dpo->tob->cur_anim_step = 1;
				dpo->state = STATE_MOVING_UP;
				add_animator(dpo, ANIM_SATAN);
				phys_set_colliding_tile_object(dpo,
						TILE_COLLISION_FULL, deadly_tile_handler, 0);
				ascii8_set_data(PAGE_SPRITES);
				spr_valloc_pattern_set(PATRN_BULLET);
			} else {
				room_objs += NEXT_OBJECT(struct map_object_movable);
				continue;
			}
			room_objs += NEXT_OBJECT(struct map_object_movable);
		}
	}

	add_jean();
	phys_set_sprite_collision_handler(jean_collision_handler);

	// show all elements
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
			spr_show(dpo->spr);
		} else if (dpo->type == DISP_OBJECT_TILE && dpo->visible) {
			tile_object_show(dpo->tob, scr_tile_buffer, false);
		}
	}

	spr_show(dpo_jean.spr);

	if (room != ROOM_EVIL_CHAMBER && room != ROOM_BONFIRE) {
		show_room_title(game_state.room);
		show_score_panel();
	}

	vdp_memcpy(vdp_base_names_grp1, scr_tile_buffer, 704);
	spr_refresh();
	vdp_screen_enable();
	start_music(room);
}
