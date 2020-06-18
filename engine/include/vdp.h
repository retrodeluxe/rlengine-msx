/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _VDP_H_
#define _VDP_H_

#define vdp_txt 0
#define vdp_grp1 1
#define vdp_grp2 2
#define vdp_mult 3

#define vdp_trasnp	0
#define vdp_black 	1
#define vdp_green	2
#define vdp_lgreen	3
#define vdp_dblue	4
#define vdp_blue 	5
#define vdp_dred	6
#define vdp_lblue	7
#define vdp_red 	8
#define vdp_lred	9
#define vdp_yellow	10
#define vdp_lyellow	11
#define vdp_dgreen	12
#define vdp_magenta	13
#define vdp_grey	14
#define vdp_white	15

#define vdp_base_names_grp1     0x1800
#define vdp_base_color_grp1     0x2000
#define vdp_base_chars_grp1     0x0000
#define vdp_base_spatr_grp1     0x1b00
#define vdp_base_sppat_grp1     0x3800

#define vdp_hw_max_sprites      32
#define vdp_hw_max_patterns     255

struct vdp_hw_sprite {
	uint8_t y;
	uint8_t x;
	uint8_t pattern;
	uint8_t color;
};

#define vdp_poke_names(OFFSET, PATTERN) 	vdp_poke(vdp_base_names_grp1 + (OFFSET), PATTERN)

extern void vdp_screen_disable(void);
extern void vdp_screen_enable(void);
extern void vdp_set_mode(char mode);
extern void vdp_set_color(char ink, char border);
extern void vdp_poke(uint16_t address, uint8_t value) __nonbanked;
extern void vdp_memset(uint16_t vaddress, uint16_t size, uint8_t value) __nonbanked;
extern void vdp_copy_to_vram(uint8_t * buffer, uint16_t vaddress, uint16_t length) __nonbanked;
extern void vdp_set_hw_sprite(struct vdp_hw_sprite *spr, uint8_t spi) __nonbanked;
extern void vdp_init_hw_sprites(char spritesize, char zoom);
extern void vdp_fastcopy_nametable(uint8_t * buffer) __nonbanked;
extern void vdp_clear_grp1(uint8_t color);
extern void vdp_print_grp1(char x, char y, char *msg);

#endif
