#ifndef _ANIM_H_
#define _ANIM_H_

enum anim_t {
	ANIM_LEFT_RIGHT,
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

extern struct animator animators[7];

void add_animator(struct displ_object *dpo, enum anim_t animidx);

void anim_static(struct displ_object *obj);
void anim_gravity(struct displ_object *obj);
void anim_left_right(struct displ_object *obj);
// void anim_horizontal_projectile(struct displ_object *obj);
void anim_joystick(struct displ_object *obj);
void anim_jump(struct displ_object *obj);
void anim_cycle_tile(struct displ_object *obj);

void dpo_animate(struct displ_object *dpo, int8_t dx, int8_t dy);

void init_animators();

#endif
