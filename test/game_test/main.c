/**
 *
 * Copyright (C) Retro DeLuxe 2017-2020, All rights reserved.
 *
 */
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "dpo.h"
#include "phys.h"
#include "list.h"
#include "font.h"
#include "pt3.h"

#include "anim.h"
#include "scene.h"
#include "logic.h"
#include "banks.h"

#include "gen/intro_map_defs.h"
#include "gen/intro_tileset_ext.h"
#include "gen/game_test_tiles_ext.h"
#include "gen/map_defs.h"
#include "gen/map_init.h"

#include "gen/big_font_upper_ext.h"
#include "gen/big_font_lower_ext.h"
#include "gen/font_big_digits_ext.h"
#include "gen/big_font_symbols_ext.h"
#include "gen/parchment_map_defs.h"

#include <stdlib.h>

void show_logo();
void show_game_over();
void show_title_screen();
void show_intro_animation();
void animate_all() __nonbanked;
void show_score_panel();
void show_parchment(uint8_t id);

void play_music() __nonbanked;
void load_room(uint8_t room, bool reload);
void init_room_titles();
void load_intro_scene();

struct tile_set logo;
struct tile_set tileset_intro;
struct tile_set tileset_gameover;
struct tile_set parchment;
struct font font_upper;
struct font font_lower;
struct font font_digits;
struct font font_symbols;
struct font_set intro_font_set;
struct animator *anim;
struct displ_object *dpo;
struct list_head *elem,*elem2;

uint8_t stick;
uint8_t trigger;

/** screen nametable buffer **/
uint8_t scr_tile_buffer[768];

/** ROM transfer buffers **/
uint8_t data_buffer[2100];
uint8_t sfx_buffer[431];

uint16_t reftick;
bool fps_stall;

extern const unsigned char title_song_pt3[];
extern const unsigned int title_song_pt3_len;
extern const unsigned char introjean_song_pt3[];
extern const unsigned int introjean_song_pt3_len;
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

#pragma CODE_PAGE 3

void main() __nonbanked
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear(0);

start:
	show_title_screen();

	init_resources();
	init_animators();

	show_intro_animation();

	init_map_tilelayers();
	init_map_object_layers();
	init_game_state();

	load_room(game_state.room, true);
	show_score_panel();

	/** game loop **/
	for(;;) {
		sys_irq_enable();
		sys_wait_vsync();

		reftick = sys_get_ticks();
		stick = sys_get_stick(0) | sys_get_stick(1);
		trigger = sys_get_trigger(0) | sys_get_trigger(1);

		change_room();
		if (game_state.show_parchment) {
			show_parchment(game_state.show_parchment);
			load_room(game_state.room, true);
			show_score_panel();
			game_state.show_parchment = 0;
		} else if (game_state.change_room) {
			load_room(game_state.room, false);
		}

		animate_all();

		/* framerate limiter 25/30fps */
		while (sys_get_ticks() - reftick < 1);
			// some work we can do in this loop?

		if (game_state.death) {
			if(--game_state.live_cnt == 0) {
				show_game_over();
				goto start;
			}
			handle_death();
			load_room(game_state.room, false);
			show_score_panel();
		}

		if (game_state.refresh_score) {
			game_state.refresh_score = false;
			show_score_panel();
		}


	}
}

void animate_all() __nonbanked
{
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		list_for_each(elem2, &dpo->animator_list) {
			anim = list_entry(elem2, struct animator, list);
			/** XXX: hack for banked function pointers **/
			ascii8_set_code(anim->page);
			anim->run(dpo);
			ascii8_restore();
		}
	}
}

void play_music() __nonbanked
{
	pt3_decode();
	pt3_play();
}

void show_logo()
{
	// TODO : add resources
	do {
	} while (sys_get_key(8) & 1);
}

void show_game_over()
{
	uint8_t x, y, ct, size;

	stop_music();
	clear_room();
	clean_state();

	vdp_screen_disable();
	vdp_clear(0);

	tile_init();
	spr_clear();

	ascii8_set_data(PAGE_INTRO);
	INIT_TILE_SET(tileset_gameover, gameover);
	tile_set_to_vram(&tileset_gameover, 1);
	size = gameover_tile_w * gameover_tile_h;

	y = 11; x = 5;
	for (ct = 0; ct < size; ct++, x++) {
		if (x > 26) {
			y++; x = 5;
		}
		vdp_poke_names(y * 32 + x, ct);
	}
	vdp_screen_enable();
	sys_sleep(3);
}

static void load_intro_scr()
{
	vdp_screen_disable();

	vdp_init_hw_sprites(SPR_SHOW_16x16, SPR_ZOOM_OFF);

	ascii8_set_data(PAGE_INTRO2_PAT);
	vdp_memcpy_vda(intropat_vda);
	vdp_memcpy_vda(introspt_vda);
	vdp_memcpy_vda(introsat_vda);

	ascii8_set_data(PAGE_INTRO2_COL);
	vdp_memcpy_vda(introcol_vda);

	vdp_screen_enable();
}

static void load_instructions_scr()
{
	vdp_screen_disable();

	spr_clear();

	ascii8_set_data(PAGE_INSTR_PAT);
	vdp_memcpy(vdp_base_chars_grp1, instr_pat, 6144);

	ascii8_set_data(PAGE_INSTR_COL);
	vdp_memcpy(vdp_base_color_grp1, instr_col, 6144);

	vdp_screen_enable();
}

void show_title_screen()
{
	uint16_t i;
	uint8_t b = 0;
	bool showing_instr = false;

	vdp_screen_disable();
	for (i = 0; i < 768; i++) {
		vdp_write(vdp_base_names_grp1 + i, b++);
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

		if (sys_get_trigger(0)) {
			if (showing_instr) {
				load_intro_scr();
				showing_instr = false;
			} else {
				load_instructions_scr();
				showing_instr = true;
			}
		}

	} while (sys_get_key(7) & 128);

	vdp_screen_disable();
	sys_irq_unregister(play_music);
	pt3_mute();
}

static void load_intro_font()
{
	ascii8_set_data(PAGE_MAPTILES);

	INIT_FONT(font_lower, big_font_lower, FONT_LOWERCASE, 29, 1, 2);
	INIT_FONT(font_upper, big_font_upper, FONT_UPPERCASE, 26, 2, 2);
	INIT_FONT(font_digits, font_big_digits, FONT_NUMERIC, 10, 1, 2);
	INIT_FONT(font_symbols, big_font_symbols, FONT_SYMBOLS, 15, 1, 2);

	font_to_vram(&font_upper, 20);
	font_to_vram(&font_lower, 128);
	font_to_vram(&font_symbols, 180);
	font_to_vram(&font_digits, 224);

	intro_font_set.upper = &font_upper;
	intro_font_set.lower = &font_lower;
	intro_font_set.numeric = &font_digits;
	intro_font_set.symbols = &font_symbols;
}

static void load_parchment_font()
{
	ascii8_set_data(PAGE_MAPTILES);

	INIT_FONT(font_lower, big_font_lower, FONT_LOWERCASE, 29, 1, 2);
	INIT_FONT(font_upper, big_font_upper, FONT_UPPERCASE, 26, 2, 2);

	font_to_vram(&font_upper, 20);
	font_to_vram(&font_lower, 128);

	intro_font_set.upper = &font_upper;
	intro_font_set.lower = &font_lower;
	intro_font_set.numeric = NULL;
	intro_font_set.symbols = NULL;
}

/**
 *  Animation of Jean being chased by templars, showing introductory text and
 *  music.
 */
void show_intro_animation() __nonbanked
{
	uint8_t i;

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

	vdp_memcpy(vdp_base_names_grp1, scr_tile_buffer, 768);

	load_intro_scene();
	vdp_screen_enable();

	ascii8_set_data(PAGE_MUSIC);

	pt3_init(introjean_song_pt3, 1);

	sys_irq_init();
	sys_irq_register(play_music);

	reftick = sys_get_ticks();

	do {
		sys_irq_enable();
		sys_wait_vsync();

		animate_all();
		trigger = sys_get_trigger(0) | sys_get_trigger(1);

	} while (!trigger);

	sys_irq_unregister(play_music);
	pt3_mute();

	vdp_screen_disable();
	font_set_vfree(&intro_font_set);
	vdp_clear(0);
}

void show_parchment(uint8_t id)
{
	uint8_t x,y;

	game_state.jean_x = dpo_jean.xpos;
	game_state.jean_y = dpo_jean.ypos;

	stop_music();
	vdp_screen_disable();
	tile_init();
	spr_clear();

	load_parchment_font();

	font_set_color_mask(&intro_font_set, 0xA);

	ascii8_set_data(PAGE_MAPTILES);
	INIT_TILE_SET(parchment, parchment_yelow);
	tile_set_to_vram(&parchment, 1);

	sys_memcpy(scr_tile_buffer, parchment_map_parchment, 768);

	vdp_fastcopy_nametable(scr_tile_buffer);
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
			font_vprintf(&intro_font_set, 7, 12,str_parchment_3_2);
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
			break;
		case 8:
			break;
	}

	do {
		sys_irq_enable();
		sys_wait_vsync();

		trigger = sys_get_trigger(0) | sys_get_trigger(1);

	} while (!trigger);

	tile_set_vfree(&parchment);
	font_set_vfree(&intro_font_set);
}
