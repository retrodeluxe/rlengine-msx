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
extern void init_sfx();
extern void init_pal();

extern VdpRGBColor palette[];

extern const uint8_t player_state[];
extern const uint8_t snow_state[];
extern const uint8_t dust_state[];
extern const uint8_t hair_state[];

#endif
