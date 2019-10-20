/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
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

#include "anim.h"
#include "scene.h"
#include "logic.h"

#include "gen/intro_tileset_ext.h"
#include "gen/game_test_tiles_ext.h"
//#include "gen/map_defs.h"

extern const unsigned char intro_map_intro_w;
extern const unsigned char intro_map_intro_h;
extern const unsigned char intro_map_intro[];

#include <stdlib.h>
void show_logo();
void show_title_screen();
void animate_all();
void check_and_change_room();
void show_score_panel();

struct tile_set logo;
struct tile_set tileset_intro;
struct font font;

uint8_t stick;
uint16_t reftick;

bool fps_stall;

struct animator *anim;
struct displ_object *dpo;
struct list_head *elem,*elem2;

uint8_t scr_tile_buffer[768];

void main()
{
	vdp_set_mode(vdp_grp2);
	vdp_set_color(vdp_white, vdp_black);
	vdp_clear_grp1(0);

	sys_irq_init();
	//show_logo();
	show_title_screen();

	//init_map_index();
	//init_resources();

	//init_animators();
	//init_game_state();
	//load_room();
	show_score_panel();
	/** game loop **/
	for(;;) {
		reftick = sys_get_ticks();
		stick = sys_get_stick(0);

		//check_and_change_room();
		//animate_all();

		/* framerate limiter 25/30fps */
		fps_stall = true;
		while (sys_get_ticks() - reftick < 2) {
			fps_stall = false;
		}
		if (fps_stall)
			log_w("fps stall!\n");
	}
}

void show_score_panel()
{
	// TODO: For this need to figure out how to solve the problem with the font
	//       the font will also be useful for other elements in the game.
	// best option is to use an MSX font instead of the original 16pixel one, either that or just
	// use the graphics and extend the size of the ROM
}

// void animate_all() {
// 	list_for_each(elem, &display_list) {
// 		dpo = list_entry(elem, struct displ_object, list);
// 		list_for_each(elem2, &dpo->animator_list) {
// 			anim = list_entry(elem2, struct animator, list);
// 			anim->run(dpo);
// 		}
// 	}
// }


void show_logo() {
	// TODO : add resources
	do {
	} while (sys_get_key(8) & 1);
}

extern void sys_set_ascii_page3(char page);

void show_title_screen()
{
	uint8_t i;
	vdp_screen_disable();

	tile_init();

	sys_set_ascii_page3(5);

	INIT_TILE_SET(tileset_intro, intro_tileset);
	tile_set_to_vram(&tileset_intro, 1);
	vdp_clear_grp1(0);
	vdp_copy_to_vram(intro_map_intro, vdp_base_names_grp1, 768);

	INIT_FONT(font, font_upper);
	font_to_vram_bank(&font, 2, 1);
	font_vprint(&font, 7, 22, "PRESS SPACE KEY~");

	vdp_screen_enable();



	do {
	} while (sys_get_key(8) & 1);
 }
