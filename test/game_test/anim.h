#ifndef _ANIM_H_
#define _ANIM_H_

enum anim_t {
	ANIM_LEFT_RIGHT,
	ANIM_LEFT_RIGHT_FLOOR,
	ANIM_UP_DOWN,
	ANIM_GRAVITY,
	ANIM_STATIC,
	ANIM_JOYSTICK,
	ANIM_JUMP,
	ANIM_CYCLE_TILE,
};

enum dpo_state_t {
	STATE_IDLE,
	STATE_JUMPING,
	STATE_ONAIR,
	STATE_ONCEILING,
	STATE_FALLING,
	STATE_LANDING,
	STATE_ONGROUND,
};


enum obj_state {
	STATE_MOVING_LEFT,
	STATE_MOVING_RIGHT,
	STATE_MOVING_DOWN,
	STATE_MOVING_UP
};

#define MAX_ANIMATORS 9

extern struct animator animators[MAX_ANIMATORS];

void add_animator(struct displ_object *dpo, enum anim_t animidx);

#endif
