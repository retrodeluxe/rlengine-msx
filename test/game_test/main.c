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
#include "pt3_nt2.h"

#include "anim.h"
#include "scene.h"
#include "logic.h"
#include "banks.h"

#include "gen/intro_tileset_ext.h"
#include "gen/game_test_tiles_ext.h"
#include "gen/map_defs.h"
#include "gen/map_init.h"

#include <stdlib.h>

void show_logo();
void show_title_screen();
void animate_all();
void check_and_change_room();
void show_score_panel();
void play_music();
void load_room(uint8_t room);

struct tile_set logo;
struct tile_set tileset_intro;
struct font font;
struct animator *anim;
struct displ_object *dpo;
struct list_head *elem,*elem2;

uint8_t stick;
uint8_t trigger;

/* buffers for graphics and audio */
uint8_t scr_tile_buffer[768];
uint8_t music_buffer[2100];
uint8_t sfx_buffer[432];

uint16_t reftick;
bool fps_stall;

extern void sys_set_ascii_page3(char page);
extern const unsigned char intro_map_intro_w;
extern const unsigned char intro_map_intro_h;
extern const unsigned char intro_map_intro[];
extern const unsigned char title_song_pt3[];
extern const unsigned int title_song_pt3_len;

void main()
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

start:
	show_title_screen();

	init_map_tilelayers();
	init_map_object_layers();

	init_resources();
	init_animators();
	init_game_state();

	load_room(game_state.room);

	/** game loop **/
	for(;;) {
		sys_irq_enable();
		sys_wait_vsync();
		reftick = sys_get_ticks();
		stick = sys_get_stick(0) | sys_get_stick(1);
		trigger = sys_get_trigger(0) | sys_get_trigger(1);

		if (change_room()) {
			load_room(game_state.room);
		}

		animate_all();

		/* framerate limiter 30/60fps */
		fps_stall = true;
		while (sys_get_ticks() - reftick < 1) {
			fps_stall = false;
		}
		if (fps_stall)
			log_w("fps stall!\n");

		if (game_state.death) {
			if(--game_state.live_cnt == 0) {
				goto start;
			}
			game_state.death = false;
			load_room(game_state.room);
		}
	}
}

void animate_all() {
	list_for_each(elem, &display_list) {
		dpo = list_entry(elem, struct displ_object, list);
		list_for_each(elem2, &dpo->animator_list) {
			anim = list_entry(elem2, struct animator, list);
			anim->run(dpo);
		}
	}
}

void play_music()
{
	pt3_decode();
	pt3_play();
}

void show_logo() {
	// TODO : add resources
	do {
	} while (sys_get_key(8) & 1);
}

void show_title_screen()
{
	uint8_t i;
	vdp_screen_disable();

	/** title screen **/
	sys_set_ascii_page3(PAGE_INTRO);

	tile_init();

	INIT_TILE_SET(tileset_intro, intro_tileset);
	INIT_FONT(font, font_upper, FONT_ALFA_UPPERCASE, 25, 1, 1);

	tile_set_to_vram(&tileset_intro, 1);
	font_to_vram_bank(&font, BANK2, 1);

	vdp_clear_grp1(0);
	vdp_copy_to_vram(intro_map_intro, vdp_base_names_grp1, 768);
	font_vprint(&font, 7, 22, "PRESS SPACE KEY~");

	vdp_screen_enable();

	/** title music **/
	sys_set_ascii_page3(PAGE_MUSIC);
	sys_memcpy(music_buffer, title_song_pt3, title_song_pt3_len);
	sys_set_ascii_page3(PAGE_CODE);

	pt3_init_notes(NT);
	pt3_init(music_buffer, 0);

	sys_irq_init();
	sys_irq_register(play_music);

	do {
		sys_wait_vsync();
	} while (sys_get_key(8) & 1);

	sys_irq_unregister(play_music);
	pt3_mute();

	vdp_clear_grp1(0);
 }
