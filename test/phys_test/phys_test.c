/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include <stdlib.h>
#include "dpo.h"
#include "phys.h"
#include "list.h"
#include "pt3.h"
#include "pt3_nt2.h"
#include "song.h"

#include "gen/phys_test.h"
#include "gen/map_init.h"

enum spr_patterns_t {
	PATRN_SMILEY,
	PATRN_BULLET,
	PATRN_SKELETON,
	PATRN_SPIDER,
	PATRN_ARROW,
	PATRN_PLANT,
	PATRN_WATERDROP,
	PATRN_MONK,
	PATRN_MONK_DEATH,
	PATRN_MAX,
};

enum anim_t {
	ANIM_LEFT_RIGHT,
	ANIM_FALLING_BULLETS,
	ANIM_HORIZONTAL_PROJECTILE,
	ANIM_UP_DOWN,
	ANIM_DROP,
	ANIM_JOYSTICK,
	ANIM_JUMP,
	ANIM_THROW_ARROW,
	ANIM_SPIT_BULLETS
};

enum obj_state {
	STATE_MOVING_LEFT,
	STATE_MOVING_RIGHT,
	STATE_MOVING_DOWN,
	STATE_MOVING_UP
};

// SpritePattern spr_pattern[PATRN_MAX];

SpriteDef enemy_sprites[32];
SpriteDef arrow_sprite;
SpriteDef bullet_sprite[2];
SpriteDef monk_sprite;
DisplayObject display_object[32];
DisplayObject dpo_arrow;
DisplayObject dpo_bullet[2];
DisplayObject dpo_monk;
List display_list;
Animator animators[9];
TileSet tileset_kv;
struct map_object_item *map_object;
List *elem, *elem2, *elem3;
Animator *anim;
DisplayObject *dpo;

uint8_t stick;
uint8_t music_ready;
struct map_object_item *room_objs;

void play_music();
void anim_up_down(DisplayObject *obj);
void anim_drop(DisplayObject *obj);
void anim_falling_bullets(DisplayObject *obj);
void anim_left_right(DisplayObject *obj);
void anim_throw_arrow(DisplayObject *obj);
void anim_spit_bullets(DisplayObject *obj);
void anim_horizontal_projectile(DisplayObject *obj);
void anim_joystick(DisplayObject *obj);
void anim_jump(DisplayObject *obj);
void animate_all();
void spr_colision_handler();
void init_monk();

uint8_t screen_buf[768];
uint16_t reftick;

void init_patterns()
{
	uint8_t two_step_state[] = {2,2};
	uint8_t single_step_state[] = {1,1};
	uint8_t three_step_state[] = {3,3};
	uint8_t bullet_state[] = {1,1};
	uint8_t plant_state[] = {2};
	uint8_t waterdrop_state[] = {3};
	uint8_t spider_state[]={2};
	uint8_t archer_state[]={2,2};

	spr_define_pattern_set(PATRN_MONK, SPR_SIZE_16x32, 1, 2, three_step_state, monk1, monk1_color);
	spr_define_pattern_set(PATRN_SMILEY, SPR_SIZE_16x16, 1, 2, two_step_state, smiley, smiley_color);
	spr_define_pattern_set(PATRN_BULLET, SPR_SIZE_16x16, 1, 2, bullet_state, bullet, bullet_color);
	spr_define_pattern_set(PATRN_SKELETON, SPR_SIZE_16x32, 1, 2, archer_state, archer_skeleton, archer_skeleton_color);
	spr_define_pattern_set(PATRN_ARROW, SPR_SIZE_16x16, 1, 2, single_step_state, arrow, arrow_color);
	spr_define_pattern_set(PATRN_PLANT, SPR_SIZE_16x16, 1, 1, plant_state, plant, plant_color);
	spr_define_pattern_set(PATRN_WATERDROP, SPR_SIZE_16x16, 1, 1, waterdrop_state, waterdrop, waterdrop_color);
	spr_define_pattern_set(PATRN_SPIDER, SPR_SIZE_16x16, 1, 1, spider_state, spider, spider_color );
	spr_define_pattern_set(PATRN_MONK_DEATH, SPR_SIZE_16x32, 1, 1, two_step_state, monk_death, monk_death_color);
}

void init_animators()
{
	animators[ANIM_LEFT_RIGHT].run = anim_left_right;
	animators[ANIM_FALLING_BULLETS].run = anim_falling_bullets;
	animators[ANIM_HORIZONTAL_PROJECTILE].run = anim_horizontal_projectile;
	animators[ANIM_UP_DOWN].run = anim_up_down;
	animators[ANIM_DROP].run = anim_drop;
	animators[ANIM_JOYSTICK].run = anim_joystick;
	animators[ANIM_JUMP].run = anim_jump;
	animators[ANIM_THROW_ARROW].run = anim_throw_arrow;
	animators[ANIM_SPIT_BULLETS].run = anim_spit_bullets;
}

static void add_animator(DisplayObject *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

/**
 * displ_add_sprite
 *   - requires display_list, spr_pattern, displ_sprites?
 */
static void add_sprite(DisplayObject *dpo, uint8_t objidx,
	enum spr_patterns_t pattidx, uint8_t x, uint8_t y)
{
	spr_valloc_pattern_set(pattidx);
	spr_init_sprite(&enemy_sprites[objidx], pattidx);
	spr_set_pos(&enemy_sprites[objidx], x, y);
	dpo->type = DISP_OBJECT_SPRITE;
	dpo->spr = &enemy_sprites[objidx];
	dpo->xpos = x;
	dpo->ypos = y;
	dpo->state = 0;
	dpo->collision_state = 0;
	INIT_LIST_HEAD(&dpo->list);
	list_add(&dpo->list, &display_list);
	INIT_LIST_HEAD(&dpo->animator_list);
}


void play_music()
{
	pt3_decode();
	pt3_play();
}

void main()
{
	uint8_t i,d, x, y;
	enum map_object_property_type spr_type;

	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear_grp1(0);
	spr_init();

	init_map_object_layers();

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	/* set tile map on screen */
	sys_memset(screen_buf,0,768);
	sys_memcpy(screen_buf, map_tilemap, 768);
	vdp_fastcopy_nametable(screen_buf);

	init_patterns();
	init_animators();

	/* build scene */
	INIT_LIST_HEAD(&display_list);
	init_monk();
	INIT_LIST_HEAD(&dpo_monk.animator_list);

	room_objs = (struct map_object_item *) map_object_layer[0];
	for (dpo = display_object, i = 0; map_object->type != 255; i++, dpo++) {
        map_object = (struct map_object_item *) room_objs;
        if (map_object->type == SPRITE) {
            spr_type = map_object->object.sprite.type;
            x = map_object->x; y = map_object->y;
            switch(spr_type) {
                case TYPE_SMILEY:
                    add_sprite(dpo, i, PATRN_SMILEY, x, y);
                    add_animator(dpo, ANIM_LEFT_RIGHT);
                    break;
                case TYPE_ARCHER_SKELETON:
                    add_sprite(dpo, i, PATRN_SKELETON, x, y);
                    add_animator(dpo, ANIM_THROW_ARROW);
                    enemy_sprites[i].cur_state = 1;
                    enemy_sprites[i].cur_anim_step = 1;
                    break;
                case TYPE_PLANT:
                    add_sprite(dpo, i, PATRN_PLANT, x, y);
                    add_animator(dpo, ANIM_SPIT_BULLETS);
                    break;
                case TYPE_WATERDROP:
                    add_sprite(dpo, i, PATRN_WATERDROP, x, y);
                    add_animator(dpo, ANIM_DROP);
                    break;
                case TYPE_SPIDER:
                    add_sprite(dpo, i, PATRN_SPIDER, x, y);
                    add_animator(dpo, ANIM_UP_DOWN);
		    dpo->state = STATE_MOVING_DOWN;
                    break;
                default:
                    continue;
            }
	    room_objs++; /* we have a single type, so this works */
        }
    }

	/* add monk */

	list_add(&animators[ANIM_JOYSTICK].list, &dpo_monk.animator_list); // joystick
	//list_add(&animators[1].list, &dpo_monk.animator_list); // gravity
	INIT_LIST_HEAD(&dpo_monk.list);
	list_add(&dpo_monk.list, &display_list);

	/* Define Projectiles */
	spr_valloc_pattern_set(PATRN_ARROW);
	spr_valloc_pattern_set(PATRN_BULLET);
	spr_init_sprite(&arrow_sprite, PATRN_ARROW);
	spr_init_sprite(&bullet_sprite[0], PATRN_BULLET);
	spr_init_sprite(&bullet_sprite[1], PATRN_BULLET);

	/* Show them all */
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, DisplayObject, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			spr_show(dpo->spr);
		}
	}

	pt3_init_notes(NT);
	pt3_init(SONG00 ,0);

	sys_irq_init();
	phys_init();
	music_ready = 0;
	sys_irq_register(play_music);

	phys_set_colliding_tile(1);
	phys_set_colliding_tile(2);
	phys_set_colliding_tile(3);
	//phys_set_sprite_collision_handler(spr_colision_handler);

	/* game loop */
	for(;;) {
		sys_wait_vsync();
		reftick = sys_get_ticks();
		stick = sys_get_stick(0);
		animate_all();
		if (sys_get_ticks() - reftick >= 2)
			log_e("stall\n");
		//vdp_fastcopy_nametable(screen_buf);
		//sys_memcpy(screen_buf, map_tilemap, 768);
	}
}

void animate_all()
{
    list_for_each(elem, &display_list) {
        dpo = list_entry(elem, DisplayObject, list);
        list_for_each(elem2, &dpo->animator_list) {
            anim = list_entry(elem2, Animator, list);
            anim->run(dpo);
        }
    }
}

void init_monk()
{
	spr_valloc_pattern_set(PATRN_MONK);
	spr_init_sprite(&monk_sprite, PATRN_MONK);
	dpo_monk.xpos = 100;
	dpo_monk.ypos = 192 - 48;
	dpo_monk.type = DISP_OBJECT_SPRITE;
	dpo_monk.state = 1;
	dpo_monk.spr = &monk_sprite;
	dpo_monk.collision_state = 0;
	spr_set_pos(&monk_sprite, dpo_monk.xpos, dpo_monk.ypos);
}

void monk_death_anim()
{
    // TODO
}


/**
 * Sprite colision handler (runs on INT context)
 */
void spr_colision_handler() {
	list_for_each(elem3, &display_list) {
		dpo = list_entry(elem3, DisplayObject, list);
		// XXX: optimize with bitwise operations
		if (((dpo->xpos > dpo_monk.xpos) && (dpo->xpos < dpo_monk.xpos + 16)) ||
			((dpo->xpos + 16 > dpo_monk.xpos) && (dpo->xpos + 16 < dpo_monk.xpos + 16))) {
			// TODO: here check for y coordinates for beter accuracy
			// and perform the animation outside interrupt
			// context.
			//monk_death_anim();
			break;
		}
	}
}

/**
 * Add arrow to the display list, including animation
 */
void throw_arrow(uint8_t xpos, uint8_t ypos)
{
	arrow_sprite.cur_state = 1;
	arrow_sprite.cur_anim_step = 0;
	INIT_LIST_HEAD(&dpo_arrow.animator_list);
	list_add(&animators[2].list, &dpo_arrow.animator_list);
	spr_set_pos(&arrow_sprite, xpos + 16, ypos + 16);
	dpo_arrow.type = DISP_OBJECT_SPRITE;
	dpo_arrow.spr = &arrow_sprite;
	dpo_arrow.xpos = xpos + 16;
	dpo_arrow.ypos = ypos + 16;
	dpo_arrow.state = 0;
	INIT_LIST_HEAD(&dpo_arrow.list);
	list_add(&dpo_arrow.list, &display_list);
	spr_show(dpo_arrow.spr);
}

/**
 * Animation for Skeleton to throw an arrow periodically
 */
void anim_throw_arrow(DisplayObject *obj)
{
    static uint8_t frame_ct;
    static uint16_t time_throw;

    if (obj->state == 0) {
        if (frame_ct++ > 60) {
            frame_ct = 0;
            obj->spr->cur_anim_step = 0;
            spr_update(obj->spr);
            obj->state = 1;
            throw_arrow(obj->xpos, obj->ypos);
            time_throw = sys_gettime_secs();
        }
    }
    if (obj->state == 1) {
        if (time_throw + 1 < sys_gettime_secs()) {
            obj->spr->cur_anim_step = 1;
            spr_update(obj->spr);
        }

        if (time_throw + 5 < sys_gettime_secs()) {
            obj->state = 0;
        }
    }
}

/**
 * Add bullets to display list
 */
void throw_bullets(uint8_t xpos, uint8_t ypos)
{
    for (int8_t i = 0; i < 2; i++) {
        bullet_sprite[i].cur_anim_step = 0;
        INIT_LIST_HEAD(&dpo_bullet[i].animator_list);
        list_add(&animators[1].list, &dpo_bullet[i].animator_list);
        spr_set_pos(&bullet_sprite[i], xpos + 8 * i, ypos - 8);
        dpo_bullet[i].type = DISP_OBJECT_SPRITE;
        dpo_bullet[i].spr = &bullet_sprite[i];
        dpo_bullet[i].xpos = xpos + 8 * i;
        dpo_bullet[i].ypos = ypos - 8;
        dpo_bullet[i].state = i;
        dpo_bullet[i].collision_state = 0;
        INIT_LIST_HEAD(&dpo_bullet[i].list);
        list_add(&dpo_bullet[i].list, &display_list);
        spr_show(dpo_bullet[i].spr);
    }
}

/**
 * Animation for plant to spit bullets periodically
 */
void anim_spit_bullets(DisplayObject *obj)
{
    static uint8_t frame_ct;
    static uint16_t time_throw;

    if (obj->state == 0) {
        if (frame_ct++ > 60) {
            frame_ct = 0;
            obj->spr->cur_anim_step = 1;
            spr_update(obj->spr);
            obj->state = 1;
            throw_bullets(obj->xpos, obj->ypos);
            time_throw = sys_gettime_secs();
        }
    }
    if (obj->state == 1) {
        if (time_throw + 1 < sys_gettime_secs()) {
            obj->spr->cur_anim_step = 0;
            spr_update(obj->spr);
        }

        if (time_throw + 2 < sys_gettime_secs()) {
            obj->state = 0;
        }
    }

}

/**
 * Jump animation
 *
 */
void anim_jump(DisplayObject *obj)
{
	static uint8_t jmp_ct;

	if (obj->state == 1) {
		jmp_ct = 5;
		obj->state = 2;
	}
	if (obj->state == 2){
		if (!is_colliding_up(obj)) {
			obj->ypos-=3;
			spr_animate(obj->spr, 0, -3);
		}
		if (--jmp_ct == 0 || is_colliding_up(obj)) {
			jmp_ct = 16;
			obj->state = 3;
		}
	}
	if (obj->state == 3) {
		if (!is_colliding_up(obj)) {
			obj->ypos-=2;
			spr_animate(obj->spr, 0, -2);
		}
		if (--jmp_ct == 0 || is_colliding_up(obj)) {
			obj->state = 4;
		}
	}
	if (obj->state == 4) {
		// wait for gravity to put us in the ground
		if (is_colliding_down(obj)) {
			list_del(&animators[6].list);
			obj->state = 0;
		}
	}
}

/**
 * Animate on user input
 */
void anim_joystick(DisplayObject *obj)
{
	int8_t dx = 0,dy = 0;

	switch(stick) {
		case STICK_LEFT:
			dx = -1;
			break;
		case STICK_RIGHT:
			dx = 1;
			break;
		case STICK_UP:
			if (obj->state == 0 && is_colliding_down(obj)) {
				list_add(&animators[6].list, &dpo_monk.animator_list);
				obj->state = 1;
			}
			break;
		case STICK_DOWN:
			// TODO: Duck
			// need change the monk sprite
			// and the dimensions to check collisions
			break;
	}

	phys_detect_tile_collisions(obj, map_tilemap, dx, dy);
	dpo_simple_animate(obj, dx, dy);
}

/**
 * Horizontal projectile that disapears on wall
 */
void anim_horizontal_projectile(DisplayObject *obj)
{
	int8_t dx = 0;

	switch (obj->state) {
		case STATE_MOVING_LEFT:
			dx = 3;
			break;
		case STATE_MOVING_RIGHT:
			dx = -3;
			break;
	}
	dpo_simple_animate(obj, dx, 0);
	phys_detect_tile_collisions(obj, map_tilemap, dx, 0);

	if (is_colliding_left(obj) || is_colliding_right(obj)) {
		spr_hide(obj->spr);
		list_del(&obj->list);
	}
}

/**
 *  Horizontal translation staying on solid ground without fall,
 *      switching direction on walls
 */
void anim_left_right(DisplayObject *obj)
{
	int8_t dx = 0;

	switch(obj->state) {
		case STATE_MOVING_LEFT:
			dx = -1;
			if (is_colliding_left(obj)
				|| !is_colliding_down(obj)) {
				obj->state = STATE_MOVING_RIGHT;
				dx = 1;
			}
			break;
		case STATE_MOVING_RIGHT:
			dx = 1;
			if (is_colliding_right(obj)
				|| !is_colliding_down(obj)) {
				obj->state = STATE_MOVING_LEFT;
				dx = -1;
			}
			break;
	}
	phys_detect_tile_collisions(obj, screen_buf, dx, 4);
	dpo_simple_animate(obj, dx, 0);
}


/**
 * Animation for water droplet
 */
void anim_drop(DisplayObject *obj) {
	static uint8_t start_y;
	static uint8_t frame_ct;

	if (obj->state == 0) {
		start_y = obj->ypos;
		frame_ct = 0;
		obj->state = 1;
	}
	// state 1 - just count
	if (obj->state == 1) {
		obj->spr->cur_anim_step = 0;
		if (frame_ct++ > 30) {
			frame_ct = 0;
			obj->state = 2;
		}
	}
	if (obj->state == 2) {
		obj->spr->cur_anim_step = 1;
		if (frame_ct++ > 30) {
			frame_ct = 0;
			obj->state = 3;
		}
	}
	if (obj->state == 3 && !is_colliding_down(obj)) {
		obj->ypos++;
		obj->spr->cur_anim_step = 2;
		spr_set_pos(obj->spr, obj->xpos, obj->ypos);
	}
	if (is_colliding_down(obj)) {
        if (frame_ct++ > 10) {
			frame_ct = 0;
			obj->state = 0;
            obj->ypos = start_y;
    		obj->spr->cur_anim_step = 0;
		} else {
            obj->ypos++;
        }
        spr_set_pos(obj->spr,obj->xpos, obj->ypos);
	}
	spr_update(obj->spr);

	phys_detect_tile_collisions(obj, map_tilemap, 0, 4);
}
/*
 * Two state vertical translation with direction switch on collision
 *
 */
void anim_up_down(DisplayObject *obj)
{
	int8_t dy;

	switch(obj->state) {
		case STATE_MOVING_UP:
			dy = -1;
			break;
		case STATE_MOVING_DOWN:
			dy = 1;
			break;
	}

	dpo_simple_animate(obj, 0, dy);
	phys_detect_tile_collisions(obj, map_tilemap, 0, dy);

	if (is_colliding_down(obj)) {
		obj->state = STATE_MOVING_UP;
	} else if (is_colliding_up(obj)) {
		obj->state = STATE_MOVING_DOWN;
	}
}

/**
 * Animation for bullets thrown up and to one side,
 *      following an inverted parabolic trajectory
 */
void anim_falling_bullets(DisplayObject *obj)
{
	static uint8_t frame_cnt_a = 0;
	static uint8_t frame_cnt_b = 0;

	int8_t dx=0, dy=0;

	if (obj->state == 1) {
		frame_cnt_a++;  // compiler issue, frame_cnt_a is not initialised to 0 but to 255
		dx = 1;
		dy = -3 + frame_cnt_a++ >> 3;
	} else if (obj->state == 0) {
		dx = -1;
		dy = -3 + frame_cnt_b++ >> 3;
	}
	dpo_simple_animate(obj, dx, dy);
	phys_detect_tile_collisions(obj, map_tilemap, dx, dy);

	if (is_colliding_down(obj)) {
		if (obj->state == 1) {
			frame_cnt_a = 0;
		} else {
			frame_cnt_b = 0;
		}
		spr_hide(obj->spr);
		list_del(&obj->list);
	}
}
