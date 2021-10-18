/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */

#ifndef _SCENE_H_
#define _SCENE_H_


extern uint8_t snow_status[20];
extern uint8_t snow_aux[20];
extern uint8_t snow_aux2[20];
extern int16_t snow_y[20];
extern SpriteDef snow_spr[20];

extern uint8_t dust_ct;

extern void show_intro();
extern void add_dust(uint16_t x, uint16_t y);
extern void load_room(uint8_t x, uint8_t y);

#endif
