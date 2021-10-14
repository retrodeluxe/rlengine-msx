/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */

#ifndef _INIT_H_
#define _INIT_H_

#include "vdp.h"
#include "blit.h"

/** BlitSets */
extern BlitSet tiles_bs;
extern BlitSet font_bs;

extern void init_gfx();
extern void init_pal();

extern VdpRGBColor palette[];

#endif
