#ifndef _MSX_DISP_LIST_H_
#define _MSX_DISP_LIST_H_

#include "list.h"

#define DISP_OBJECT_SPRITE 1
#define DISP_OBJECT_GFX 2

struct displ_object;

struct animator {
	struct list_head list;
	void (*run)(struct displ_object *obj);
};

struct displ_object {
	uint8_t type;
	uint8_t state;
	int16_t xpos;
	int16_t ypos;
	uint8_t collision_state;
	struct spr_sprite_def *spr;
	struct list_head list;
	struct list_head animator_list;
};

#endif
