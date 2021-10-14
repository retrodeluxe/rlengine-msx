/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */

#ifndef _ANIM_H_
#define _ANIM_H_

#include "celeste.h"

#define DX 2
#define DY_JUMP 8
#define DX_JUMP 4
#define DY_FALL 1

#define MAX_SINE 128

extern Animator animators[MAX_ANIMATORS];
extern Animator *anim;

extern void init_animators();
extern void add_animator(DisplayObject *dpo, enum anim_t animidx);

#endif
