/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _MSX_H_PHYS
#define _MSX_H_PHYS

#include "dpo.h"
#include "tile.h"

#define COLLISION_LEFT 1
#define COLLISION_RIGHT 2
#define COLLISION_UP 4
#define COLLISION_DOWN 8
#define COLLISION_DOWN_FT 16 // fallthrough

/**
 * Check if a DisplayObject is colliding on the left side
 */
#define is_colliding_left(x) (((x)->collision_state & COLLISION_LEFT) != 0)
/**
 * Check if a DisplayObject is colliding on the right side
 */
#define is_colliding_right(x) (((x)->collision_state & COLLISION_RIGHT) != 0)
/**
 * Check if a DisplayObject is colliding on the bottom side
 */
#define is_colliding_down(x) (((x)->collision_state & COLLISION_DOWN) != 0)
/**
 * Check if a DisplayObject is colliding on the bottom side with fallthrough tiles
 */
#define is_colliding_down_ft(x)                                                \
  (((x)->collision_state & COLLISION_DOWN_FT) != 0)
/**
 * Check if a DisplayObject is colliding on the top side
 */
#define is_colliding_up(x) (((x)->collision_state & COLLISION_UP) != 0)
/**
 * Check if a DisplayObject is colliding on the x-axis
 */
#define is_colliding_x(x) (is_colliding_left((x)) || is_colliding_right((x)))
/**
 * Check if a DisplayObject is colliding on the y-axis
 */
#define is_colliding_y(x) (is_colliding_up((x)) || is_colliding_down((x)))
/**
 * Check if a DisplayObject is colliding in any direction
 */
#define is_colliding(x) (((x)->collision_state) != 0)

/*
 * Max number of TilleCollisionDefs
 */
#define MAX_COLLISION_DEFS 12

/**
 * Defines the type of tile collision
 */
typedef enum {
  /**
   * Collision in all directions
   */
  TILE_COLLISION_FULL = 1,
  /**
   * Collision only when moving down (falling)
   */
  TILE_COLLISION_DOWN = 2,
  /**
   * Collision that triggers a callback but does not impair movement
   */
  TILE_COLLISION_TRIGGER = 4,
  /**
   * Collision from multiple instances of the same objects
   * but potentially with different callbacks
   */
  TILE_COLLISION_MULTIPLE = 8,

} TileCollisionType;

/**
 * Defines a TileCollisionHandler
 */
typedef struct TileCollisionHandler TileCollisionHandler;

/**
 * Contains a TileCollisionHandler
 */
struct TileCollisionHandler {
  /**
   * ascii8 ROM page where the handler is locatted
   */
  uint8_t page;
  /**
   * handler function pointer
   */
  void (*handler)(DisplayObject *dpo, uint8_t data);
};

/**
 * Defines a set of tiles that generate collision events
 */
typedef struct TileCollisionDef TileCollisionDef;
/**
 * Contains TileCollisionDef data
 */
struct TileCollisionDef {
  /**
   * Starting tile index that defines the collision range
   */
  uint8_t start;
  /**
   * Ending tile index that defines the collision range
   */
  uint8_t end;
  /**
   * Data to be passed to the handler on collision event
   */
  uint8_t data;
  /**
   * Type of collision
   */
  TileCollisionType type;
  /**
   * DisplayObject to be passed to the handler on collision
   */
  DisplayObject *dpo;
  /**
   * Handler to be called on collision event
   */
  TileCollisionHandler callback;
};


void phys_init();
void phys_set_sprite_collision_handler(void(*handler)());
void phys_clear_sprite_collision_handler() __nonbanked;
void phys_set_tile_collision_handler(TileCollisionType type,
                              DisplayObject *dpo,
                              TileCollisionHandler *callback,
                              uint8_t data);
void phys_set_colliding_tile_object(DisplayObject *dpo,
                              TileCollisionType type,
                              TileCollisionHandler *callback,
                              uint8_t data);
void phys_set_masked_colliding_tile_object(DisplayObject *dpo,
                              TileCollisionType type,
                              uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                              TileCollisionHandler *callback,
                              uint8_t data);
void phys_clear_colliding_tile_object(DisplayObject *dpo);
void phys_set_colliding_tile(uint8_t tile);
void phys_set_down_colliding_tile(uint8_t tile);
void phys_set_trigger_colliding_tile(uint8_t tile);
void phys_clear_colliding_tile(uint8_t tile);
void phys_detect_tile_collisions(DisplayObject *obj, uint8_t *map, int8_t dx,
                                 int8_t dy, bool duck, bool notify) __nonbanked;
void phys_set_colliding_tile_set(TileSet *ts);

#endif
