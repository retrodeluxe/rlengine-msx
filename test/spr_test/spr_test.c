/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include <stdlib.h>

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "log.h"
#include "gen/spr_test.h"

/**
 * Global data is placed in 0xC000 (RAM page 2) in 32K roms by default
 */

SpriteDef monk;
SpriteDef kvalley;
SpriteDef bat;
SpriteDef death_spr;
SpriteDef bee[10];
SpriteDef rats[10];

/**
 * NOTE : any initialized global data must be constant.
 */
const uint8_t control_patt[8] = {255,255,255,255,255,255,255,255};
const uint8_t control_colors [1] = {6};

int16_t xpos[10], ypos[10];
int16_t xpos2[10], ypos2[10];

void test_16x16_3layers();
void test_16x32_2layers();
void test_32x16_1layers();
void test_32x32_1layer();
void test_5th_sprite();
void test_16x16_mode2();

void main()
{
	unsigned int count = 0;

	vdp_set_mode(MODE_GRP1);
	vdp_set_color(COLOR_WHITE, COLOR_BLUE);
	vdp_clear(0);

  test_16x16_3layers();
  test_16x32_2layers();
  test_32x16_1layers();
  test_32x32_1layer();
  test_5th_sprite();

  // sprite mode 2
  vdp_set_mode(MODE_GRP3);
	vdp_set_color(COLOR_WHITE, COLOR_BLUE);
	vdp_clear(0);

  test_16x16_mode2();

	do {
	} while (sys_get_keyrow(8) & 1);
}

/*
 *
 */
void test_16x16_3layers() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  spr_load_defs();
  spr_valloc_pattern_set(SPR_VALLEY);

  xpos[0] = 200; ypos [0] = 100;
  spr_init_sprite(&kvalley, SPR_VALLEY);
  spr_set_pos(&kvalley, xpos[0], ypos[0]);
  spr_show(&kvalley);

  sys_irq_init();

  dir = -1; ctr = 0;
  do {
    sys_wait_vsync();
    spr_refresh();
    spr_animate(&kvalley,dir,0);
    xpos[0] += dir; ctr++;
    if (xpos[0] < -16) xpos[0] = 255;
    if (xpos[0] > 255) xpos[0] = -16;
    if (ctr > 300) {
      ctr = 0;
      dir *= -1;
    }
    spr_set_pos(&kvalley, xpos[0], ypos[0]);
    spr_update(&kvalley);
  } while (sys_get_keyrow(8) & 1);
}

/**
 *
 */
void test_16x32_2layers() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  spr_load_defs();
  spr_valloc_pattern_set(SPR_MONK);

  xpos[0] = 50; ypos [0] = 100;
  spr_init_sprite(&monk, SPR_MONK);
  spr_set_pos(&monk, xpos[0], ypos[0]);
  spr_show(&monk);

  dir = -1; ctr = 0;
  do {
  	sys_wait_vsync();
  	spr_refresh();
  	spr_animate(&monk,dir,0);
    xpos[0] += dir; ctr++;
    if (xpos[0] < -16) xpos[0] = 255;
    if (xpos[0] > 255) xpos[0] = -16;
    if (ctr > 300) {
      ctr = 0;
      dir *= -1;
    }
  	spr_set_pos(&monk, xpos[0], ypos[0]);
  	spr_update(&monk);
  } while (sys_get_keyrow(8) & 1);
}

/**
 *
 */
void test_32x16_1layers() {
  int8_t dir;
  uint16_t ctr;
  spr_init();
  spr_load_defs();
  spr_valloc_pattern_set(SPR_DARKBAT);

  xpos[0] = 20; ypos [0] = 100;
  spr_init_sprite(&bat, SPR_DARKBAT);
  spr_set_pos(&bat, xpos[0], ypos[0]);
  spr_show(&bat);

  dir = -1; ctr = 0;
  do {
  	sys_wait_vsync();
  	spr_refresh();
  	spr_animate(&bat,dir,0);
    xpos[0] += dir; ctr++;
    if (xpos[0] < -32) xpos[0] = 255;
    if (xpos[0] > 255) xpos[0] = -32;
    if (ctr > 300) {
      ctr = 0;
      dir *= -1;
    }
  	spr_set_pos(&bat, xpos[0], ypos[0]);
  	spr_update(&bat);
  } while (sys_get_keyrow(8) & 1);
}

/**
 *
 */
void test_32x32_1layer() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  spr_load_defs();
  spr_valloc_pattern_set(SPR_DEATH);

  xpos[0] = 50; ypos [0] = 100;
  spr_init_sprite(&death_spr, SPR_DEATH);
  spr_set_pos(&death_spr, xpos[0], ypos[0]);
  spr_show(&death_spr);

  dir = -1; ctr = 0;
  do {
  	sys_wait_vsync();
  	spr_refresh();
  	spr_animate(&death_spr,dir,0);
    xpos[0] += dir; ctr++;
    if (xpos[0] < -32) xpos[0] = 255;
    if (xpos[0] > 255) xpos[0] = -32;
    if (ctr > 300) {
      ctr = 0;
      dir *= -1;
    }
  	spr_set_pos(&death_spr, xpos[0], ypos[0]);
  	spr_update(&death_spr);
  } while (sys_get_keyrow(8) & 1);
 }

/**
 * Test 5th sprite interleaving
 */
void test_5th_sprite() {
  int i, dir1, dir2;
  spr_init();
  /**
   * Single layer sprites with animation in two directions
   */

  spr_load_defs();
  spr_valloc_pattern_set(SPR_BEE);
  spr_valloc_pattern_set(SPR_RAT);

  for (i = 0; i< 6; i++) {
    spr_init_sprite(&bee[i], SPR_BEE);
    spr_init_sprite(&rats[i], SPR_RAT);
    xpos[i] = i * 20; ypos[i] = 70;
    xpos2[i] = 100 + i * 20; ypos2[i] = 150;
    spr_set_pos(&bee[i], xpos[i], ypos[i]);
    spr_set_pos(&rats[i], xpos2[i], ypos2[i]);
    spr_show(&bee[i]);
    spr_show(&rats[i]);
  }

  dir1 = 1; dir2 = -1;
  do {
    sys_wait_vsync();
    spr_refresh();
    for (i = 0; i< 6; i++) {
      spr_animate(&bee[i],1,-1);
      spr_animate(&rats[i],-1,1);
      spr_set_pos(&bee[i], xpos[i], ypos[i]);
      spr_set_pos(&rats[i], xpos2[i], ypos2[i]);
      ypos[i] += dir1;
      ypos2[i] += dir2;
      if (ypos[i] < 0 || ypos[i] > 160) dir1 *= -1;
      if (ypos2[i] < 0 || ypos2[i] > 160) dir2 *= -1;
      spr_update(&bee[i]);
      spr_update(&rats[i]);
    }
  } while (sys_get_keyrow(8) & 1);

}

void test_16x16_mode2() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  spr_load_defs();
  vdp_init_hw_sprites(SPR_SIZE_16, SPR_ZOOM_ON);
  spr_valloc_pattern_set(SPR_VALLEY2);

  xpos[0] = 200; ypos [0] = 100;
  spr_init_sprite(&kvalley, SPR_VALLEY2);
  spr_set_pos(&kvalley, xpos[0], ypos[0]);
  spr_show(&kvalley);

  sys_irq_init();

  dir = -1; ctr = 0;
  do {
    sys_wait_vsync();
    spr_refresh();
    spr_animate(&kvalley,dir,0);
    xpos[0] += dir; ctr++;
    if (xpos[0] < -16) xpos[0] = 255;
    if (xpos[0] > 255) xpos[0] = -16;
    if (ctr > 300) {
      ctr = 0;
      dir *= -1;
    }
    spr_set_pos(&kvalley, xpos[0], ypos[0]);
    spr_update(&kvalley);
  } while (sys_get_keyrow(8) & 1);
}
