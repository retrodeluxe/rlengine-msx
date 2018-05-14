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
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include <stdlib.h>
#include "displ.h"
#include "phys.h"
#include "list.h"

#include "gen/phys_test.h"

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
	ANUM_UP_DOWN,
	ANIM_DROP,
	ANIM_JOYSTICK,
    ANIM_JUMP,
    ANIM_THROW_ARROW,
    ANIM_SPIT_BULLETS
};

struct spr_pattern_set spr_pattern[PATRN_MAX];

struct spr_sprite_def enemy_sprites[32];
struct spr_sprite_def arrow_sprite;
struct spr_sprite_def bullet_sprite[2];
struct spr_sprite_def monk_sprite;
struct displ_object display_object[32];
struct displ_object dpo_arrow;
struct displ_object dpo_bullet[2];
struct displ_object dpo_monk;
struct list_head display_list;
struct animator animators[9];
struct tile_set tileset_kv;
struct map_object_item *map_object;
struct list_head *elem, *elem2, *elem3;
struct animator *anim;
struct displ_object *dpo;

struct work_struct animation_work;

uint8_t map_buf[768];
uint8_t stick;

void anim_up_down(struct displ_object *obj);
void anim_drop(struct displ_object *obj);
void anim_falling_bullets(struct displ_object *obj);
void anim_left_right(struct displ_object *obj);
void anim_throw_arrow(struct displ_object *obj);
void anim_spit_bullets(struct displ_object *obj);
void anim_horizontal_projectile(struct displ_object *obj);
void anim_joystick(struct displ_object *obj);
void anim_jump(struct displ_object *obj);
void animate_all();
void spr_colision_handler();
void init_monk();


void init_patterns()
{
    SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SMILEY], SPR_SIZE_16x16, 1, 2, 2, smiley);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_BULLET], SPR_SIZE_16x16, 1, 2, 2, bullet);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SKELETON], SPR_SIZE_16x32, 1, 2, 2, archer_skeleton);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_ARROW], SPR_SIZE_16x16, 1, 2, 1, arrow);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_PLANT], SPR_SIZE_16x16, 1, 1, 2, plant);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_WATERDROP], SPR_SIZE_16x16, 1, 1, 3, waterdrop);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_SPIDER], SPR_SIZE_16x16, 1, 1, 2, spider);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_MONK], SPR_SIZE_16x32, 1, 2, 3, monk1);
	SPR_DEFINE_PATTERN_SET(spr_pattern[PATRN_MONK_DEATH], SPR_SIZE_16x32, 1, 1, 2, monk_death);
}

void init_animators()
{
    animators[ANIM_LEFT_RIGHT].run = anim_left_right;
    animators[ANIM_FALLING_BULLETS].run = anim_falling_bullets;
    animators[ANIM_HORIZONTAL_PROJECTILE].run = anim_horizontal_projectile;
    animators[ANUM_UP_DOWN].run = anim_up_down;
    animators[ANIM_DROP].run = anim_drop;
    animators[ANIM_JOYSTICK].run = anim_joystick;
    animators[ANIM_JUMP].run = anim_jump;
    animators[ANIM_THROW_ARROW].run = anim_throw_arrow;
    animators[ANIM_SPIT_BULLETS].run = anim_spit_bullets;
}

static void add_animator(struct displ_object *dpo, enum anim_t animidx)
{
	list_add(&animators[animidx].list, &dpo->animator_list);
}

static void add_sprite(struct displ_object *dpo, uint8_t objidx, enum spr_patterns_t pattidx)
{
	spr_valloc_pattern_set(&spr_pattern[pattidx]);
	spr_init_sprite(&enemy_sprites[objidx], &spr_pattern[pattidx]);
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
}

void main()
{
	uint8_t i,d;

	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);
	spr_init();

    init_map_object_layers();

	INIT_TILE_SET(tileset_kv, kingsvalley);
	tile_set_to_vram(&tileset_kv, 1);

	/* set tile map on screen */
	vdp_copy_to_vram(map_tilemap, vdp_base_names_grp1, 768);

    init_patterns();
    init_animators();

	/* build scene */
	INIT_LIST_HEAD(&display_list);
	for (dpo = display_object, i = 0; i < map_objects_size; i++, dpo++) {
        map_object = (struct map_object_item *) map_object_objects[i];
		if (map_object->type == SPRITE) {
			if (map_object->object.sprite.type == TYPE_SMILEY) {
                add_sprite(dpo, i, PATRN_SMILEY);
                add_animator(dpo, ANIM_LEFT_RIGHT);
			} else if (map_object->object.sprite.type == TYPE_ARCHER_SKELETON) {
                add_sprite(dpo, i, PATRN_SKELETON);
                add_animator(dpo, ANIM_THROW_ARROW);
				enemy_sprites[i].cur_dir = 1;
                enemy_sprites[i].cur_anim_step = 1;
			} else if (map_object->object.sprite.type == TYPE_PLANT) {
                add_sprite(dpo, i, PATRN_PLANT);
                add_animator(dpo, ANIM_SPIT_BULLETS);
			} else if (map_object->object.sprite.type == TYPE_WATERDROP) {
                add_sprite(dpo, i, PATRN_WATERDROP);
                add_animator(dpo, ANIM_DROP);
			} else if (map_object->object.sprite.type == TYPE_SPIDER) {
                add_sprite(dpo, i, PATRN_SPIDER);
                add_animator(dpo, ANUM_UP_DOWN);
			} else {
				continue;
			}
		}
	}

    /* Add monk */
	init_monk();
	INIT_LIST_HEAD(&dpo_monk.animator_list);
	list_add(&animators[ANIM_JOYSTICK].list, &dpo_monk.animator_list); // joystick
	//list_add(&animators[1].list, &dpo_monk.animator_list); // gravity
	INIT_LIST_HEAD(&dpo_monk.list);
	list_add(&dpo_monk.list, &display_list);

    /* Define Projectiles */
    spr_valloc_pattern_set(&spr_pattern[PATRN_ARROW]);
    spr_valloc_pattern_set(&spr_pattern[PATRN_BULLET]);
    spr_init_sprite(&arrow_sprite, &spr_pattern[PATRN_ARROW]);
    spr_init_sprite(&bullet_sprite[0], &spr_pattern[PATRN_BULLET]);
    spr_init_sprite(&bullet_sprite[1], &spr_pattern[PATRN_BULLET]);

    /* Show them all */
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		if (dpo->type == DISP_OBJECT_SPRITE) {
			spr_show(dpo->spr);
		}
	}

	sys_irq_init();
	phys_init();

	phys_set_colliding_tile(1);
    phys_set_colliding_tile(2);
    phys_set_colliding_tile(3);
	//phys_set_sprite_collision_handler(spr_colision_handler);

	/* game loop */
	do {
		stick = sys_get_stick(0);
		animate_all();
	} while (sys_get_key(8) & 1);
}

void animate_all() {
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		list_for_each(elem2, &dpo->animator_list) {
			anim = list_entry(elem2, struct animator, list);
			anim->run(dpo);
		}
	}
}

void init_monk()
{
    spr_valloc_pattern_set(&spr_pattern[PATRN_MONK]);
	spr_init_sprite(&monk_sprite, &spr_pattern[PATRN_MONK]);
	dpo_monk.xpos = 100;
	dpo_monk.ypos = 192 - 48;
	dpo_monk.type = DISP_OBJECT_SPRITE;
	dpo_monk.state = 0;
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
		dpo = list_entry(elem3, struct displ_object, list);
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
    arrow_sprite.cur_dir = 1;
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
void anim_throw_arrow(struct displ_object *obj)
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
void anim_spit_bullets(struct displ_object *obj)
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
void anim_jump(struct displ_object *obj)
{
	static uint8_t jmp_ct;

	if (obj->state == 1) {
		jmp_ct = 5;
		obj->state = 2;
	}
	if (obj->state == 2){
		if (!is_colliding_up(obj)) {
			obj->ypos-=3;
			spr_animate(obj->spr, 0, -3 ,0);
		}
		if (--jmp_ct == 0 || is_colliding_up(obj)) {
			jmp_ct = 16;
			obj->state = 3;
		}
	}
	if (obj->state == 3) {
		if (!is_colliding_up(obj)) {
			obj->ypos-=2;
			spr_animate(obj->spr, 0, -2 ,0);
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
void anim_joystick(struct displ_object *obj)
{
    int8_t dx = 0,dy = 0;

	switch(stick) {
		case STICK_LEFT:
			if (!is_colliding_left(obj)) {
                dx = -1;
                obj->xpos+=dx;
                spr_animate(obj->spr, dx, dy ,0);
			}
			break;
		case STICK_RIGHT:
			if (!is_colliding_right(obj)) {
                dx = 1;
                obj->xpos+=dx;
                spr_animate(obj->spr, dx, dy ,0);
			}
			break;
		case STICK_UP:
            // FIXME: handle this properly
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

    phys_detect_tile_collisions(obj, map_tilemap, dx, dy, 0);
}

/**
 * Horizontal projectile that disapears on wall
 */
void anim_horizontal_projectile(struct displ_object *obj)
{
    int8_t dx = 0;
	if (obj->state == 0 && !is_colliding_right(obj)) {
        dx = 3;
	} else if (obj->state == 1 && !is_colliding_left(obj)) {
        dx =-3;
	}
    obj->xpos+=dx;
    spr_animate(obj->spr, dx, 0, 0);

    phys_detect_tile_collisions(obj, map_tilemap, dx, 0, 0);

    if (is_colliding_left(obj) || is_colliding_right(obj)) {
        spr_hide(obj->spr);
        list_del(&obj->list);
    }
}

/**
 *  Horizontal translation staying on solid ground without fall,
 *      switching direction on walls
 */
void anim_left_right(struct displ_object *obj)
{
    int8_t dx = 0;

	if (obj->state == 0 && !is_colliding_right(obj)) {
        dx = 1;
	} else if (obj->state == 1 && !is_colliding_left(obj)) {
        dx = -1;
	}

    // avoid walking on the air
    if (!is_colliding_down(obj)) {
        if (obj->state == 0)
            dx = -1;
        else
            dx = 1;
    }

    obj->xpos+=dx;
    spr_animate(obj->spr, dx, 0, 0);

    if (is_colliding_left(obj)) {
        obj->state = 0;
    } else if (is_colliding_right(obj)) {
        obj->state = 1;
    } else if (obj->state == 0 && !is_colliding_down(obj)) {
        obj->state = 1;
    } else if (obj->state == 1 && !is_colliding_down(obj)) {
        obj->state = 0;
    }

    phys_detect_tile_collisions(obj, map_tilemap, dx, 1,0 );
}


/**
 * Animation for water droplet
 */
void anim_drop(struct displ_object *obj) {
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

    phys_detect_tile_collisions(obj, map_tilemap, 0, 1, 0);
}
/*
 * Two state vertical translation with direction switch on collision
 *
 */
void anim_up_down(struct displ_object *obj)
{
    int8_t dy;

    if (is_colliding_down(obj)) {
		obj->state = 0;
	} else if (is_colliding_up(obj)) {
		obj->state = 1;
	}

	if (obj->state == 0 && !is_colliding_up(obj)) {
		dy= -1;
	} else if (obj->state == 1 && !is_colliding_down(obj)) {
		dy = 1;
	}
    obj->ypos+=dy;
    spr_animate(obj->spr, 0, dy, 0);

    phys_detect_tile_collisions(obj, map_tilemap, 0, dy, 4);
}

/**
 * Animation for bullets thrown up and to one side,
 *      following an inverted parabolic trajectory
 */
void anim_falling_bullets(struct displ_object *obj)
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
    obj->ypos+=dy;
    obj->xpos+=dx;
    spr_animate(obj->spr, dx, dy, 0);

    phys_detect_tile_collisions(obj, map_tilemap, dx, dy, 2);
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
