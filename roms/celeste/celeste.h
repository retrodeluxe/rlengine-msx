/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */
#ifndef _CELESTE_H_
#define _CELESTE_H_

#define NUM_SNOW_SMALL 14
#define NUM_SNOW_BIG 4

#define LEVEL_IDX = (room.x % 8 + room.y * 8)
#define SCENE_MAX_DPO 40

enum map_tile_t {
    TYPE_PLAYER_SPAWN = 1,
    TYPE_PLAYER,
    TYPE_PLATFORM,
    TYPE_KEY = 9,
    TYPE_PLATFORM_A = 12,
    TYPE_PLATFORM_B,
    TYPE_VERTICAL_SPIKES = 18,
    TYPE_SPRING = 19,
    TYPE_CHEST = 21,
    TYPE_BALLOON = 23,
    TYPE_FALL_FLOOR = 24,
    TYPE_FRUIT = 25,
    TYPE_FLY_FRUIT = 29,
    TYPE_FAKE_WALL = 65,
    TYPE_MESSAGE = 87,
    TYPE_BIG_CHEST = 97,
    TYPE_FLAG = 119,
    TYPE_SMOKE,
    TYPE_ORB,
    TYPE_LIFEUP
};

enum map_object_t {
    TILE_SPIKES,
    TILE_FAKE_WALL,
    TILE_CHEST,
    TILE_KEY,
    TILE_SPRING,
    TILE_FALL_FLOOR,
    TILE_OBJECT_MAX
};

enum spr_patterns_t {
    PATRN_PLAYER,
    PATRN_SNOW_SMALL,
    PATRN_SNOW_BIG,
    PATRN_DUST,
    PATRN_BALLOON,
    PATRN_CLOUD,
    PATRN_HAIR_BIG,
    PATRN_HAIR_SMALL,
    PATRN_MAX
};

enum anim_t {
    ANIM_PLAYER,
    ANIM_PLAYER_SPAWN,
    ANIM_SNOW,
    ANIM_FAST_SNOW,
    ANIM_DUST,
    ANIM_SHAKE,
    MAX_ANIMATORS,
};

enum player_state {
    STATE_IDLE,
    STATE_MOVING_LEFT,
    STATE_MOVING_RIGHT,
    STATE_JUMPING,
    STATE_CROUCHING,
    STATE_LOOKING_UP,
    STATE_FALLING,
    STATE_COLLISION,
    STATE_DEATH,
};

/* this needs to match sprite definition */
enum player_anim_state {
    PLAYER_ANIM_RIGHT,
    PLAYER_ANIM_HANG_RIGHT,
    PLAYER_ANIM_CROUCH_RIGHT,
    PLAYER_ANIM_UP_RIGHT,
    PLAYER_ANIM_UP_LEFT,
    PLAYER_ANIM_CROUCH_LEFT,
    PLAYER_ANIM_HANG_LEFT,
    PLAYER_ANIM_LEFT,
};

extern uint8_t scr_buffer[256];

extern uint8_t col_buffer[1024];

/** map data */
extern const uint8_t MAP_DATA[8192];

/** main display list */
extern List display_list;

/* dpo iterator */
extern List *elem, *elem2;
extern DisplayObject *dpo;

extern uint16_t reftick;

extern const uint8_t player_state[];
extern const uint8_t snow_state[];

extern uint8_t stick, trigger_a, trigger_b;
extern uint8_t y_offset;

extern void sys_set_rom();
extern void sys_set_bios();
extern void sys_memcpy_rom(uint8_t *dst, uint8_t *src, uint16_t size);

#endif
