#ifndef _MSX_DISP_LIST_H_
#define _MSX_DISP_LIST_H_

#include "list.h"
#include "sprite.h"

#define DISP_OBJECT_SPRITE 1
#define DISP_OBJECT_TILE 2

struct displ_object;

struct animator {
	struct list_head list;
	void (*run)(struct displ_object *obj);
};

struct displ_object {
	uint8_t type; 	/*< sprite or dynamic tile */

	/* static animation data */
	int8_t max;	/*<  max coordinate */
	int8_t min;	/*<  min coordinate */
	int8_t speed;	/*<  speed */
	int8_t color;

	/* dynamic animation data */
	uint8_t state;
	int16_t xpos;
	int16_t ypos;
	int8_t vy;
	int8_t vx;
	uint8_t collision_state;
	struct spr_sprite_def *spr;
	struct tile_object *tob;
	struct list_head list;
	struct list_head animator_list;
};

#endif
