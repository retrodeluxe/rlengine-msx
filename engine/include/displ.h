#ifndef _MSX_DISP_LIST_H_
#define _MSX_DISP_LIST_H_

#include "list.h"

#define DISP_OBJECT_SPRITE 1
#define DISP_OBJECT_GFX 2

struct display_object;

struct animator {
	struct list_head list;
	void (*run)(struct display_object *obj);
};

struct display_object {
	uint8_t type;
	uint8_t state;
	uint8_t xpos;
	uint8_t ypos;
	uint8_t collision_state;
	struct spr_sprite_def *spr;
	//struct animator *animator;
	// fix this mess
	struct list_head animator_list;
};

#endif
