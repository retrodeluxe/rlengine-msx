/**
 *
 * Copyright (C) Retro DeLuxe 2017-2020, All rights reserved.
 *
 */
#include "dpo.h"
#include "font.h"
#include "list.h"
#include "log.h"
#include "map.h"
#include "msx.h"
#include "phys.h"
#include "pt3.h"
#include "sfx.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"
#include "ascii8.h"

#include "anim.h"
#include "banks.h"
#include "logic.h"
#include "scene.h"

#include "gen/game_test_tiles_ext.h"
#include "gen/map_defs.h"
#include "gen/map_init.h"

#include "gen/big_font_lower_ext.h"
#include "gen/big_font_symbols_ext.h"
#include "gen/big_font_upper_ext.h"
#include "gen/font_big_digits_ext.h"
#include "gen/parchment_map_defs.h"

#include <stdlib.h>

void show_game_over();
void show_title_screen();
void show_retrodeluxe_logo();
void show_intro_animation() __nonbanked;
void animate_all() __nonbanked;
void animate_jean() __nonbanked;
void show_score_panel();
void refresh_score();
void show_parchment(uint8_t id);

void play_music() __nonbanked;
void stop_music() __nonbanked;
void load_room(uint8_t room, bool reload);
void init_room_titles();
void load_intro_scene() __nonbanked;
void change_room() __nonbanked;
void show_ending_animation();
static void reload_font_digits();

extern void init_resources();
extern void init_animators();
extern void init_game_state();
extern void handle_death();
extern void clear_room();

TileSet logo;
TileSet tileset_intro;
TileSet tileset_ending;
TileSet tileset_gameover;
TileSet parchment;
Font font_upper;
Font font_lower;
Font font_digits;
Font font_symbols;
FontSet intro_font_set;
Animator *anim;
DisplayObject *dpo;
List *elem, *elem2;
SpriteDef score_hearth_mask;
SpriteDef score_cross_mask;

uint8_t stick;
uint8_t trigger;

/** screen nametable buffer **/
uint8_t scr_tile_buffer[768];

/** ROM transfer buffers **/
uint8_t data_buffer[2100];
uint8_t sfx_buffer[431];

/** score panel primitives **/
TileObject score;
Font big_digits;
FontSet score_font_set;

/** current song **/
uint8_t *current_song;

bool init_music;
bool muted;
uint8_t play_frame;

uint8_t v_frame;

uint16_t reftick;
bool fps_stall;

extern TileSet tileset_room_title[ROOM_MAX];
extern TileSet tileset[TILE_MAX];

extern const unsigned char title_song_pt3[];
extern const unsigned int title_song_pt3_len;
extern const unsigned char introjean_song_pt3[];
extern const unsigned int introjean_song_pt3_len;
extern const unsigned char prayerofhope_song_pt3[];
extern const unsigned int prayerofhope_song_pt3_len;
extern const unsigned int NT[];

extern const char str_press_space[];
extern const char instr_col[];
extern const char instr_pat[];
extern const char intropat_vda[];
extern const char introcol_vda[];
extern const char introspt_vda[];
extern const char introsat_vda[];
extern const char str_intro_1[];
extern const char str_intro_2[];
extern const char str_intro_3[];
extern const char str_intro_5[];
extern const char str_intro_4[];
extern const char str_parchment_1_1[];
extern const char str_parchment_1_2[];
extern const char str_parchment_2_1[];
extern const char str_parchment_2_2[];
extern const char str_parchment_3_1[];
extern const char str_parchment_3_2[];
extern const char str_parchment_4_1[];
extern const char str_parchment_4_2[];
extern const char str_parchment_5_1[];
extern const char str_parchment_5_2[];
extern const char str_parchment_6_1[];
extern const char str_parchment_6_2[];
extern const char str_parchment_7_1[];
extern const char str_parchment_7_2[];
extern const char str_parchment_7_3[];
extern const char str_parchment_8_1[];
extern const char str_parchment_8_2[];
extern const char str_parchment_8_3[];
extern const char str_ending_1[];
extern const char str_ending_2[];
extern const char str_ending_3[];
extern const char str_ending_4[];

#pragma CODE_PAGE 3

void main() __nonbanked {
  sys_memset((uint8_t *)0xC000, 0, 0x2000);

  vdp_set_mode(MODE_GRP2);
  vdp_set_color(COLOR_WHITE, COLOR_BLACK);
  vdp_clear(0);

  sys_disable_kbd_click();
  sys_rand_init((uint8_t *)&main);

  show_retrodeluxe_logo();
start:
  init_music = false;
  current_song = NULL;
  show_title_screen();

  init_resources();
  init_animators();

  show_intro_animation();

  init_map_tilelayers();
  init_map_object_layers();
  init_game_state();
  init_scene();

  load_room(game_state.room, true);

  /** game loop **/
  for (;;) {
    sys_irq_enable();
    sys_wait_vsync();
    spr_refresh();
    // debug
    // vdp_memcpy(VRAM_BASE_NAME, scr_tile_buffer, 704);

    reftick = sys_get_ticks();
    stick = sys_get_stick(0) | sys_get_stick(1);
    trigger = sys_get_trigger(0) | sys_get_trigger(1);

    change_room();
    if (game_state.start_ending_seq) {
      game_state.change_room = true;
      game_state.room = ROOM_EVIL_CHAMBER;
      game_state.start_ending_seq = false;
    }
    if (game_state.start_bonfire_seq) {
      game_state.change_room = true;
      game_state.room = ROOM_BONFIRE;
      game_state.start_bonfire_seq = false;
    }
    if (game_state.show_parchment) {
      show_parchment(game_state.show_parchment);
      game_state.jean_state = STATE_IDLE;
      load_room(game_state.room, true);
      game_state.show_parchment = 0;
    } else if (game_state.final_animation) {
      show_ending_animation();
      goto start;
    } else if (game_state.change_room) {
      load_room(game_state.room, false);
      spr_refresh();
      // hack: ensure font digits not overwritten by pentagram
      if (game_state.room == ROOM_SATAN || game_state.room == ROOM_HAGMAN_TREE)
        reload_font_digits();
    } else if (game_state.teletransport) {
      load_room(game_state.room, true);
    }

    /* adjust enemy speed at 60Hz */
    if (sys_is60Hz())
      v_frame = ++v_frame % 6;
    else
      v_frame = 1;
    if (v_frame)
      animate_all();
    animate_jean();

    /* framerate limiter to 25/30fps */
    fps_stall = true;
    while (sys_get_ticks() - reftick < 1) {
      fps_stall = false;
    }

    if (game_state.death) {
      if (--game_state.live_cnt == 0) {
        show_game_over();
        goto start;
      }
      handle_death();
      load_room(game_state.room, true);
      // if (game_state.room == ROOM_SATAN)
      //	reload_font_digits();
      refresh_score();
    }

    if (game_state.refresh_score) {
      game_state.refresh_score = false;
      refresh_score();
    }
  }
}

void animate_all() __nonbanked {
  list_for_each(elem, &display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    list_for_each(elem2, &dpo->animator_list) {
      anim = list_entry(elem2, Animator, list);
      /** XXX: hack for banked function pointers **/
      // log_e("dpo aidx:%d n:%d\n",dpo->spr->aidx,
      // dpo->spr->pattern_set->planes);
      ascii8_set_code(anim->page);
      anim->run(dpo);
      ascii8_restore();
    }
  }
}

void animate_jean() __nonbanked {
  list_for_each(elem2, &dpo_jean.animator_list) {
    anim = list_entry(elem2, Animator, list);
    ascii8_set_code(anim->page);
    anim->run(&dpo_jean);
    ascii8_restore();
  }
}

void play_music() __nonbanked {
  if (sys_is60Hz()) {
    play_frame = ++play_frame % 6;
  } else {
    play_frame = 1;
  }
  if (play_frame != 0) {
    pt3_decode();
    pt3_play();
  }
}

void show_retrodeluxe_logo() {
  uint8_t i, x, y;

  tile_init();
  sys_irq_init();

  ascii8_set_data(PAGE_INTRO);
  INIT_TILE_SET(logo, retro_logo);
  tile_set_to_vram(&logo, 0);

  i = 0;
  for (y = 0; y < retro_logo_tile_h; y++)
    for (x = 0; x < retro_logo_tile_w; x++) {
      vdp_write(VRAM_BASE_NAME + 11 + 9 * 32 + x + y * 32, i++);
      sys_sleep_ms(50);
    }

  sys_sleep(3);
}

void show_game_over() {
  uint8_t x, y, ct, size;

  stop_music();
  clear_room();

  vdp_screen_disable();
  vdp_clear(0);

  tile_init();
  spr_clear();
  spr_refresh();

  ascii8_set_data(PAGE_INTRO);
  INIT_TILE_SET(tileset_gameover, gameover);
  tile_set_to_vram(&tileset_gameover, 1);
  size = gameover_tile_w * gameover_tile_h;

  y = 11;
  x = 5;
  for (ct = 0; ct < size; ct++, x++) {
    if (x > 26) {
      y++;
      x = 5;
    }
    vdp_poke_names(y * 32 + x, ct);
  }
  vdp_screen_enable();
  sys_sleep(5);
}

static void load_intro_scr() {
  vdp_screen_disable();

  vdp_init_hw_sprites(SPR_SIZE_16, SPR_ZOOM_OFF);

  ascii8_set_data(PAGE_INTRO2_PAT);
  vdp_memcpy_vda(intropat_vda);
  vdp_memcpy_vda(introspt_vda);
  vdp_memcpy_vda(introsat_vda);

  ascii8_set_data(PAGE_INTRO2_COL);
  vdp_memcpy_vda(introcol_vda);

  vdp_screen_enable();
}

static void load_instructions_scr() {
  vdp_screen_disable();

  spr_clear();
  spr_refresh();

  ascii8_set_data(PAGE_INSTR_PAT);
  vdp_memcpy(VRAM_BASE_PTRN, instr_pat, 6144);

  ascii8_set_data(PAGE_INSTR_COL);
  vdp_memcpy(VRAM_BASE_COLR, instr_col, 6144);

  vdp_screen_enable();
}

void show_title_screen() {
  uint16_t i;
  uint8_t b = 0;
  bool showing_instr = false;

  vdp_screen_disable();
  for (i = 0; i < 768; i++) {
    vdp_write(VRAM_BASE_NAME + i, b++);
  }
  load_intro_scr();

  ascii8_set_data(PAGE_INTRO);
  pt3_init_notes(NT);

  ascii8_set_data(PAGE_MUSIC);
  sys_memcpy(data_buffer, title_song_pt3, title_song_pt3_len);
  pt3_init(data_buffer, 1);

  sys_irq_init();
  sys_irq_register(play_music);

  do {
    sys_wait_vsync();

    if (sys_get_trigger(0) | sys_get_trigger(1) | sys_get_trigger(2)) {
      if (showing_instr) {
        load_intro_scr();
        showing_instr = false;
      } else {
        load_instructions_scr();
        showing_instr = true;
      }
    }
    trigger = sys_get_trigger(3) | sys_get_trigger(4);

  } while (!trigger && (sys_get_key(7) & 128));

  trigger = 0;
  vdp_screen_disable();
  sys_irq_unregister(play_music);
  pt3_mute();
}

static void load_intro_font() {
  ascii8_set_data(PAGE_MAPTILES);

  INIT_FONT(font_lower, big_font_lower, FONT_LOWERCASE, 29, 1, 2);
  INIT_FONT(font_upper, big_font_upper, FONT_UPPERCASE, 26, 2, 2);
  INIT_FONT(font_digits, font_big_digits, FONT_NUMERIC, 10, 1, 2);
  INIT_FONT(font_symbols, big_font_symbols, FONT_SYMBOLS, 15, 1, 2);

  font_to_vram(&font_upper, 1);
  font_to_vram(&font_lower, 128);
  font_to_vram(&font_symbols, 180);
  font_to_vram(&font_digits, 224);

  intro_font_set.upper = &font_upper;
  intro_font_set.lower = &font_lower;
  intro_font_set.numeric = &font_digits;
  intro_font_set.symbols = &font_symbols;
}

static void load_parchment_font() {
  ascii8_set_data(PAGE_MAPTILES);

  INIT_FONT(font_lower, big_font_lower, FONT_LOWERCASE, 29, 1, 2);
  INIT_FONT(font_upper, big_font_upper, FONT_UPPERCASE, 26, 2, 2);
  INIT_FONT(font_symbols, big_font_symbols, FONT_SYMBOLS, 15, 1, 2);

  font_to_vram(&font_upper, 17);
  font_to_vram(&font_lower, (uint8_t)(128 + 17));
  font_to_vram(&font_symbols, (uint8_t)(180 + 17));

  intro_font_set.upper = &font_upper;
  intro_font_set.lower = &font_lower;
  intro_font_set.numeric = NULL;
  intro_font_set.symbols = &font_symbols;
}

static void reload_font_digits() {
  ascii8_set_data(PAGE_MAPTILES);
  INIT_FONT(big_digits, font_big_digits, FONT_NUMERIC, 10, 1, 2);
  font_to_vram_bank(&big_digits, BANK2, 224);
}

/**
 *  Animation of Jean being chased by templars, showing introductory text and
 *  music.
 */
void show_intro_animation() __nonbanked {
  tile_init();
  vdp_screen_disable();
  vdp_clear(0);
  load_intro_font();

  sys_memset(scr_tile_buffer, 0, 768);
  font_printf(&intro_font_set, 1, 1, scr_tile_buffer, str_intro_1);
  font_printf(&intro_font_set, 1, 3, scr_tile_buffer, str_intro_2);
  font_printf(&intro_font_set, 1, 5, scr_tile_buffer, str_intro_3);
  font_printf(&intro_font_set, 4, 20, scr_tile_buffer, str_intro_4);
  font_printf(&intro_font_set, 1, 22, scr_tile_buffer, str_intro_5);

  vdp_memcpy(VRAM_BASE_NAME, scr_tile_buffer, 768);

  load_intro_scene();
  spr_refresh();
  vdp_screen_enable();

  ascii8_set_data(PAGE_MUSIC);

  pt3_init(introjean_song_pt3, 1);

  sys_irq_init();
  sys_irq_register(play_music);

  do {
    sys_irq_enable();
    sys_wait_vsync();
    spr_refresh();
    reftick = sys_get_ticks();

    animate_all();
    while (sys_get_ticks() - reftick < 1);

    trigger = sys_get_trigger(0) | sys_get_trigger(1);

  } while (!trigger);

  sys_irq_unregister(play_music);
  pt3_mute();

  vdp_screen_disable();
  font_set_vfree(&intro_font_set);
  vdp_clear(0);
}

void show_ending_gate_animation(uint8_t frame) {
  uint8_t *dst, *dst1, *base = scr_tile_buffer + 256 + 32;
  uint8_t x, y, tile, tile1, w;

  // clear compiler warning
  tile = 0; w =0; dst = base; tile1 = 1; dst1 = dst;

  switch (frame) {
  case 0:
    tile = 1;
    dst = base + 14;
    w = 4;
    break;
  case 1:
    tile = 5;
    dst = base + 14;
    w = 4;
    break;
  case 2:
    tile = 9;
    dst = base + 13;
    w = 6;
    break;
  case 3:
    tile = 15;
    dst = base + 13;
    w = 6;
    break;
  case 4:
    tile = 21;
    tile1 = 23;
    dst = base + 12;
    dst1 = base + 18;
    w = 2;
    break;
  case 5:
    tile = 7;
    tile1 = 5;
    dst = base + 12;
    dst1 = base + 18;
    w = 2;
    break;
  }
  switch (frame) {
  case 0:
  case 1:
  case 2:
  case 3:
    for (y = 0; y < 6; y++) {
      for (x = 0; x < w; x++) {
        *(dst++) = tile++;
      }
      tile += 24 - w;
      dst += 32 - w;
    }
    break;
  case 4:
  case 5:
    for (y = 0; y < 6; y++) {
      for (x = 0; x < w; x++) {
        *(dst++) = tile++;
        *(dst1++) = tile1++;
      }
      tile += 24 - w;
      dst += 32 - w;
      tile1 += 24 - w;
      dst1 += 32 - w;
    }
    break;
  }
}

void show_ending_animation() {
  uint8_t i;

  spr_clear();
  spr_refresh();
  tile_init();
  vdp_screen_disable();
  vdp_clear(0);
  load_parchment_font();

  sys_memset(scr_tile_buffer, 0, 768);
  font_printf(&intro_font_set, 6, 2, scr_tile_buffer, str_ending_1);
  font_printf(&intro_font_set, 10, 4, scr_tile_buffer, str_ending_2);
  font_printf(&intro_font_set, 5, 19, scr_tile_buffer, str_ending_3);
  font_printf(&intro_font_set, 8, 21, scr_tile_buffer, str_ending_4);

  tile_init();
  ascii8_set_data(PAGE_INTRO);
  INIT_TILE_SET(tileset_ending, ending);
  tile_set_to_vram_bank(&tileset_ending, BANK1, 1);

  show_ending_gate_animation(0);
  vdp_memcpy(VRAM_BASE_NAME, scr_tile_buffer, 768);

  vdp_screen_enable();

  ascii8_set_data(PAGE_MUSIC);

  pt3_init(prayerofhope_song_pt3, 1);

  sys_irq_init();
  sys_irq_register(play_music);

  reftick = sys_get_ticks();

  i = 1;
  do {
    sys_irq_enable();
    sys_wait_vsync();

    sys_sleep_ms(700);
    show_ending_gate_animation(i);
    vdp_memcpy(VRAM_BASE_NAME, scr_tile_buffer, 768);

  } while (i++ < 10);

  sys_sleep(5);
  sys_irq_unregister(play_music);
  pt3_mute();

  vdp_screen_disable();
  font_set_vfree(&intro_font_set);
  vdp_clear(0);
}

void show_parchment(uint8_t id) {

  game_state.jean_x = dpo_jean.xpos;
  game_state.jean_y = dpo_jean.ypos;

  vdp_screen_disable();
  tile_init();
  spr_clear();

  load_parchment_font();

  if (id < 7) {
    font_set_color_mask(&intro_font_set, 0xA);

    ascii8_set_data(PAGE_MAPTILES);
    INIT_TILE_SET(parchment, parchment_yelow);
    tile_set_to_vram(&parchment, 1);
  } else if (id == 7) {
    font_set_color_mask(&intro_font_set, 0x8);

    ascii8_set_data(PAGE_MAPTILES);
    INIT_TILE_SET(parchment, parchment_red);
    tile_set_to_vram(&parchment, 1);
  } else if (id == 8) {
    font_set_color_mask(&intro_font_set, 0x7);

    ascii8_set_data(PAGE_MAPTILES);
    INIT_TILE_SET(parchment, parchment_blue);
    tile_set_to_vram(&parchment, 1);
  }

  sys_memcpy(scr_tile_buffer, parchment_map_parchment, 768);

  vdp_fastcopy_nametable(scr_tile_buffer);
  spr_refresh();
  vdp_screen_enable();

  switch (id) {
  case 2:
    font_vprintf(&intro_font_set, 8, 9, str_parchment_1_1);
    font_vprintf(&intro_font_set, 8, 12, str_parchment_1_2);
    break;
  case 1:
    font_vprintf(&intro_font_set, 8, 9, str_parchment_2_1);
    font_vprintf(&intro_font_set, 8, 12, str_parchment_2_2);
    break;
  case 4:
    font_vprintf(&intro_font_set, 9, 9, str_parchment_3_1);
    font_vprintf(&intro_font_set, 7, 12, str_parchment_3_2);
    break;
  case 3:
    font_vprintf(&intro_font_set, 7, 9, str_parchment_4_1);
    font_vprintf(&intro_font_set, 7, 12, str_parchment_4_2);
    break;
  case 6:
    font_vprintf(&intro_font_set, 8, 9, str_parchment_5_1);
    font_vprintf(&intro_font_set, 6, 12, str_parchment_5_2);
    break;
  case 5:
    font_vprintf(&intro_font_set, 8, 9, str_parchment_6_1);
    font_vprintf(&intro_font_set, 8, 12, str_parchment_6_2);
    break;
  case 7:
    font_vprintf(&intro_font_set, 6, 8, str_parchment_7_1);
    font_vprintf(&intro_font_set, 9, 11, str_parchment_7_2);
    font_vprintf(&intro_font_set, 7, 14, str_parchment_7_3);
    break;
  case 8:
    font_vprintf(&intro_font_set, 6, 8, str_parchment_8_1);
    font_vprintf(&intro_font_set, 5, 11, str_parchment_8_2);
    font_vprintf(&intro_font_set, 7, 14, str_parchment_8_3);
    break;
  }

  do {
    sys_irq_enable();
    sys_wait_vsync();

    trigger = sys_get_trigger(0) | sys_get_trigger(1);

  } while (!trigger);

  trigger = 0;
  tile_set_vfree(&parchment);
  font_set_vfree(&intro_font_set);
  reload_font_digits();
}

void show_room_title(uint8_t room) {
  uint8_t i, tile, w;
  uint16_t vram_offset;

#define MAX_TITLE_LEN 18
#define SCR_WIDTH 32

  if (room == ROOM_EVIL_CHAMBER || room == ROOM_BONFIRE)
    return;

  if (room == ROOM_SATAN) {
    ascii8_set_data(PAGE_SPRITES);
  } else {
    ascii8_set_data(PAGE_ROOM_TITLES);
  }

  w = tileset_room_title[room].w;
  vram_offset = VRAM_BASE_NAME + SCR_WIDTH + 22 * SCR_WIDTH;

  tile_set_to_vram_bank(&tileset_room_title[room], BANK2, 184);

  vdp_memset(vram_offset - MAX_TITLE_LEN, MAX_TITLE_LEN, 0);
  vdp_memset(vram_offset - MAX_TITLE_LEN + SCR_WIDTH, MAX_TITLE_LEN, 0);

  tile = 184;
  for (i = 0; i < w; i++) {
    vdp_write(vram_offset - w + i, tile + i);
    vdp_write(vram_offset - w + i + SCR_WIDTH, tile + i + w);
  }

  // we can free after use
  tile_set_vfree(&tileset_room_title[room]);
}

void refresh_score() {
  char snum[3];

  /** clear up 2 digit live count **/
  vdp_write(VRAM_BASE_NAME + 3 + 22 * 32, 0);
  vdp_write(VRAM_BASE_NAME + 3 + 23 * 32, 0);
  vdp_write(VRAM_BASE_NAME + 7 + 22 * 32, 0);
  vdp_write(VRAM_BASE_NAME + 7 + 23 * 32, 0);

  score_font_set.numeric = &big_digits;
  _itoa(game_state.live_cnt, snum, 10);
  font_vprintf(&score_font_set, 2, 22, snum);
  _itoa(game_state.cross_cnt, snum, 10);
  font_vprintf(&score_font_set, 6, 22, snum);
}

void show_score_panel() {
  SpritePattern *ps = &spr_pattern[PATRN_HEARTH_MASK];

  ascii8_set_data(PAGE_DYNTILES);
  tile_set_to_vram_bank(&tileset[TILE_HEART_STATUS], BANK2, (uint8_t)(252 - 4));
  tile_set_to_vram_bank(&tileset[TILE_CROSS_STATUS], BANK2, (uint8_t)(252 - 8));

  score.y = (uint8_t)(192 - 16);
  score.x = 0;
  score.state = 0;
  score.frame = 0;
  score.tileset = &tileset[TILE_HEART_STATUS];
  score.idx = 0;

  tile_object_show(&score, scr_tile_buffer, true);

  // add mask for hearth
  ascii8_set_data(PAGE_SPRITES);
  spr_valloc_pattern_set(PATRN_HEARTH_MASK);
  ps->colors2[0] = 1;

  spr_init_sprite(&score_hearth_mask, PATRN_HEARTH_MASK);
  spr_set_pos(&score_hearth_mask, score.x, score.y);
  spr_show(&score_hearth_mask);

  score.x = 32;
  score.y = (uint8_t)(192 - 16);
  score.state = 0;
  score.frame = 0;
  score.tileset = &tileset[TILE_CROSS_STATUS];
  score.idx = 0;

  tile_object_show(&score, scr_tile_buffer, true);

  // add mask for cross
  ascii8_set_data(PAGE_SPRITES);
  spr_valloc_pattern_set(PATRN_CROSS_MASK);
  ps = &spr_pattern[PATRN_CROSS_MASK];
  ps->colors2[0] = 1;

  spr_init_sprite(&score_cross_mask, PATRN_CROSS_MASK);
  spr_set_pos(&score_cross_mask, score.x, score.y);
  spr_show(&score_cross_mask);

  refresh_score();
}

void play_room_music() __nonbanked {
  if (sys_is60Hz()) {
    play_frame = ++play_frame % 6;
  } else {
    play_frame = 1;
  }
  if (play_frame != 0) {
    if (!muted)
      pt3_decode();
    sfx_play();
    pt3_play();
  }
}

void stop_music() __nonbanked {
  pt3_mute();
  muted = true;
}

void start_music2() __nonbanked {
  pt3_init(data_buffer, 1);
  if (!init_music) {
    sys_irq_register(play_room_music);
    init_music = true;
  }
  muted = false;
}

void start_music(uint8_t room) {
  uint8_t *new_song;
  uint16_t new_song_len;

  ascii8_set_data(PAGE_MUSIC);

  new_song = NULL;
  new_song_len = 0;
  switch (room) {
  case ROOM_EVIL_CHAMBER:
  case ROOM_FOREST:
  case ROOM_GRAVEYARD:
    new_song = huntloop_song_pt3;
    new_song_len = huntloop_song_pt3_len;
    break;
  case ROOM_CHURCH_ENTRANCE:
  case ROOM_CHURCH_ALTAR:
  case ROOM_CHURCH_TOWER:
  case ROOM_CHURCH_WINE_SUPPLIES:
  case ROOM_CATACOMBS:
  case ROOM_CATACOMBS_FLIES:
  case ROOM_CATACOMBS_WHEEL:
    new_song = church_song_pt3;
    new_song_len = church_song_pt3_len;
    break;
  case ROOM_PRAYER_OF_HOPE:
    new_song = prayerofhope_song_pt3;
    new_song_len = prayerofhope_song_pt3_len;
    break;
  case ROOM_CAVE_LAKE:
  case ROOM_CAVE_DRAGON:
  case ROOM_CAVE_GHOST:
  case ROOM_CAVE_GATE:
  case ROOM_CAVE_TUNNEL:
  case ROOM_HIDDEN_GARDEN:
  case ROOM_HIDDEN_RIVER:
    new_song = cave_song_pt3;
    new_song_len = cave_song_pt3_len;
    break;
  case ROOM_EVIL_CHURCH:
  case ROOM_EVIL_CHURCH_2:
  case ROOM_EVIL_CHURCH_3:
    new_song = hell_song_pt3;
    new_song_len = hell_song_pt3_len;
    break;
  case ROOM_SATAN:
  case ROOM_DEATH:
    ascii8_set_data(PAGE_INTRO);
    new_song = evilfight_song_pt3;
    new_song_len = evilfight_song_pt3_len;
    break;
  case ROOM_BONFIRE:
  case ROOM_HAGMAN_TREE:
    new_song = NULL;
    break;
  }

  if (new_song != current_song) {
    if (new_song != NULL) {
      stop_music();
      sys_memcpy(data_buffer, new_song, new_song_len);
      start_music2();
    } else {
      stop_music();
      muted = true;
    }
    current_song = new_song;
  } else {
    muted = false;
  }
}
