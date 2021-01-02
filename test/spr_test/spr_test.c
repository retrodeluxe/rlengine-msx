/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "log.h"
#include "gen/spr_test.h"
#include <stdlib.h>

/**
 * Global data is placed in 0xC000 (RAM page 2) in 32K roms by default
 */

 enum spr_patterns_t {
 	PATRN_BEE,
 	PATRN_RAT,
 	PATRN_MONK,
  PATRN_BAT,
  PATRN_DEATH,
  PATRN_KVALLEY
 };

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

const uint8_t kv_states[] = {4,4};
const uint8_t monk_states[] = {3,3};
const uint8_t two_states[] = {2,2};
const uint8_t two_states2[] = {3,3};
const uint8_t four_states[] = {3,3,3,3};

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
	} while (sys_get_key(8) & 1);
}

/*
 *
 */
void test_16x16_3layers() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  SPR_DEFINE_PATTERN_SET(PATRN_KVALLEY, SPR_SIZE_16x16, 3, 2, kv_states, valley2);
  spr_valloc_pattern_set(PATRN_KVALLEY);

  xpos[0] = 200; ypos [0] = 100;
  spr_init_sprite(&kvalley, PATRN_KVALLEY);
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
  } while (sys_get_key(8) & 1);
}

/**
 *
 */
void test_16x32_2layers() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  SPR_DEFINE_PATTERN_SET(PATRN_MONK, SPR_SIZE_16x32, 2, 2, monk_states, monk1);
  spr_valloc_pattern_set(PATRN_MONK);

  xpos[0] = 50; ypos [0] = 100;
  spr_init_sprite(&monk, PATRN_MONK);
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
  } while (sys_get_key(8) & 1);
}

/**
 *
 */
void test_32x16_1layers() {
  int8_t dir;
  uint16_t ctr;
  spr_init();
  SPR_DEFINE_PATTERN_SET(PATRN_BAT, SPR_SIZE_32x16, 1, 2, two_states, darkbat);
  spr_valloc_pattern_set(PATRN_BAT);

  xpos[0] = 20; ypos [0] = 100;
  spr_init_sprite(&bat, PATRN_BAT);
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
  } while (sys_get_key(8) & 1);
}

/**
 *
 */
void test_32x32_1layer() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  SPR_DEFINE_PATTERN_SET(PATRN_DEATH, SPR_SIZE_32x32, 1, 2, two_states2, death);
  spr_valloc_pattern_set(PATRN_DEATH);

  xpos[0] = 50; ypos [0] = 100;
  spr_init_sprite(&death_spr, PATRN_DEATH);
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
  } while (sys_get_key(8) & 1);
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

  SPR_DEFINE_PATTERN_SET(PATRN_BEE, SPR_SIZE_16x16, 1, 2,
    two_states, bee1);
  SPR_DEFINE_PATTERN_SET(PATRN_RAT, SPR_SIZE_16x16, 1, 2,
    two_states, rat);

  spr_valloc_pattern_set(PATRN_BEE);
  spr_valloc_pattern_set(PATRN_RAT);

  for (i = 0; i< 6; i++) {
    spr_init_sprite(&bee[i], PATRN_BEE);
    spr_init_sprite(&rats[i], PATRN_RAT);
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
  } while (sys_get_key(8) & 1);

}

void test_16x16_mode2() {
  int8_t dir;
  uint16_t ctr;

  spr_init();
  vdp_init_hw_sprites(SPR_SIZE_16, SPR_ZOOM_ON);
  SPR_DEFINE_PATTERN_SET(PATRN_KVALLEY, SPR_SIZE_16x16, 3, 2, kv_states, valley3);
  spr_valloc_pattern_set(PATRN_KVALLEY);

  xpos[0] = 200; ypos [0] = 100;
  spr_init_sprite(&kvalley, PATRN_KVALLEY);
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
  } while (sys_get_key(8) & 1);
}
