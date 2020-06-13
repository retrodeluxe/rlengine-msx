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

#include <stdlib.h>

void show_logo();
void show_game_over();
void show_title_screen();
void animate_all() __nonbanked;
void show_score_panel();
void play_music() __nonbanked;
void load_room(uint8_t room);
void init_room_titles();

struct tile_set logo;
struct tile_set tileset_intro;
struct tile_set tileset_gameover;
struct font font;
struct animator *anim;
struct displ_object *dpo;
struct list_head *elem,*elem2;

uint8_t stick;
uint8_t trigger;

/** screen nametable buffer **/
uint8_t scr_tile_buffer[768];

/** ROM transfer buffers **/
uint8_t data_buffer[2100];

uint16_t reftick;
bool fps_stall;
extern const unsigned char title_song_pt3[];
extern const unsigned int title_song_pt3_len;
extern const unsigned int NT[];

#pragma CODE_PAGE 3
extern const char debug1[];
void main() __nonbanked
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

start:
	show_title_screen();

	init_map_tilelayers();
	init_map_object_layers();
	init_room_titles();

	init_resources();
	init_animators();
	init_game_state();

	load_room(game_state.room);
	show_score_panel();

	/** game loop **/
	for(;;) {
		sys_irq_enable();
		sys_wait_vsync();

		reftick = sys_get_ticks();
		stick = sys_get_stick(0) | sys_get_stick(1);
		trigger = sys_get_trigger(0) | sys_get_trigger(1);

		change_room();
		if (game_state.change_room){
			load_room(game_state.room);
		}

		animate_all();

		/* framerate limiter 30/60fps */
		fps_stall = true;
		while (sys_get_ticks() - reftick < 1) {
			fps_stall = false;
		}
		// if (fps_stall)
		// 	log_w("fps stall!\n");

		if (game_state.death) {
			if(--game_state.live_cnt == 0) {
				show_game_over();
				goto start;
			}
			handle_death();
			load_room(game_state.room);
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
			ascii8_set_code(6);
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
	vdp_clear_grp1(0);

	tile_init();
	spr_init();

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

extern const char str_press_space[];

void show_title_screen()
{
	uint8_t i, color;

	vdp_screen_disable();
	ascii8_set_data(PAGE_INTRO);

	tile_init();
	INIT_TILE_SET(tileset_intro, intro_tileset);
	tile_set_to_vram(&tileset_intro, 1);

	INIT_FONT(font, font_upper, FONT_ALFA_UPPERCASE, 26, 1, 1);
	font_to_vram_bank(&font, BANK2, 1);

	vdp_clear_grp1(0);
	vdp_copy_to_vram(intro_map_intro, vdp_base_names_grp1, 768);

	font_vprint(&font, 7, 22, str_press_space);

	vdp_screen_enable();

	/** title music **/
	pt3_init_notes(NT);  // NT is in PAGE_INTRO

	ascii8_set_data(PAGE_MUSIC);

	pt3_init(title_song_pt3, 0);

	sys_irq_init();
	sys_irq_register(play_music);

	do {
		sys_wait_vsync();
	} while (sys_get_key(8) & 1);

	sys_irq_unregister(play_music);
	pt3_mute();

	vdp_clear_grp1(0);
 }
