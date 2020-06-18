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

#include "msx.h"
#include "vdp.h"

#pragma CODE_PAGE 2

static char t_spritesize;

void vdp_screen_disable(void)
{
	__asm
	call 0x0041
	__endasm;
}

void vdp_screen_enable(void)
{
	__asm
	call 0x0044
	__endasm;
}

void vdp_set_mode(char mode)
{
	mode;

	__asm
	push	ix
	push	iy
	ld	a,4(ix)
	ld	(0xfcaf),a
	ld	ix,#0x5f
	ld	iy,(0xfcc0)
	call	0x1c
	pop iy
	pop ix
	__endasm;
}

void vdp_set_color(char ink, char border)
{
	ink;
	border;

	__asm
	ld	a,4(ix)
	ld	b,5(ix)
	sla	a
	sla	a
	sla	a
	sla	a
	add	a,b
	di
	out	(#0x99),a
	ld	a,#0x87
	ei
	out	(#0x99),a
	__endasm;
}


void vdp_write(uint16_t address, uint8_t value) __nonbanked
{
	address;
	value;

	__asm
	di
	ld	l,4(ix)
	ld	h,5(ix)
	ld	a,l
	di
	out	(0x99),a
	ld	a,h
	add	a,#0x40
	ei
	out	(0x99),a
	ld	a,6(ix)
	out	(0x98),a
	__endasm;
}

void vdp_memset(uint16_t vaddress, uint16_t size, uint8_t value) __nonbanked
{
	vaddress;
	size;
	value;

	__asm
	ld	l,4(ix)
	ld	h,5(ix)
	in	a,(0x99)
	ld	a,l
	di
	out	(0x99),a
	ld	a,h
	add	a,#0x40
	out	(0x99),a
	ld	e,6(ix)
	ld	d,7(ix)
	ld	a,8(ix)
	ld	b,e
	dec	de
	inc	d
$1:
	out	(0x98),a
	djnz	$1
	dec	d
	jr	nz,$1
	ei
	__endasm;
}

/**
 * copy ram buffer to vram
 */
void vdp_memcpy(uint16_t vaddress, uint8_t *buffer, uint16_t size) __nonbanked
{
	buffer;
	vaddress;
	size;

	__asm
	ld	l,4(ix)
	ld	h,5(ix)
	ld	a,l
	di
	out	(0x99),a
	ld	a,h
	add	a,#0x40
	out	(0x99),a
	ld	l,6(ix)
	ld	h,7(ix)
	ld	e,8(ix)
	ld	d,9(ix)
	ld	c,#0x98
	ld	b,e
	dec	de
	inc	d
$2:
	outi
	jp	nz,$2
	dec	d
	jr	nz,$2
	ei
	__endasm;
}

void vdp_set_hw_sprite(struct vdp_hw_sprite *spr, uint8_t spi) __nonbanked
{
	spr;
	spi;

	__asm
	ld	c,6(ix)	;spi
	sla	c
	sla	c
	xor	a
	ld	b,a
	ld 	hl,#vdp_base_spatr_grp1
	add	hl,bc
	ld	a,l
	di
	out	(0x99),a
	ld	a,h
	add	a,#0x40
	out	(0x99),a
	ld	l,4(ix) ; buffer address (spr)
	ld	h,5(ix)
	ld	c,#0x98
	ld	b,#4
$6:
	outi
	jp	nz,$6
	ei
	__endasm;
}

void vdp_init_hw_sprites(char spritesize, char zoom)
{
	t_spritesize = spritesize;
	zoom;

	__asm
	ld	b,#0x00
	ld	a,4(ix)
	and	#0x0F
	cp	#0x08
	jr	z,$3
	set	1,b
$3:
	ld	a,5(ix)
	cp	#0x00
	jr	z,$4
	set	0,b
$4:
	ld	hl,#0xf3e0
	ld	a,(hl)
	and	#0xfc
	or	b
	ld	(hl),a
	di
	out	(0x99),a
	ld	a,#0x81
	ei
	out	(0x99),a
	__endasm;
}

void vdp_fastcopy_nametable(uint8_t *buffer) __nonbanked
{
	buffer;

	__asm
	ld	hl,#vdp_base_names_grp1
	ld	a,l
	di
	out	(0x99),a
	ld	a,h
	add	a,#0x40
	out	(0x99),a
	ld	b,#0		; 256*3 = 768 blocks
	ld	l,4(ix)  	; buffer address
	ld	h,5(ix)
	ld	c,#0x98
$5:
	outi
	nop
	nop
	nop
	outi
	nop
	nop
	nop
	outi
	nop
	nop
	jp	nz,$5
	ei
	__endasm;
}

/*
 * Set all names to zero, then clear colors of char zero for
 *  all pattern banks
 */
void vdp_clear_grp1(uint8_t color)
{
	vdp_memset(vdp_base_names_grp1, 256 * 3, 0);
	vdp_memset(vdp_base_color_grp1, 8, color);
	vdp_memset(vdp_base_color_grp1 + 0x800, 8, color);
	vdp_memset(vdp_base_color_grp1 + 0x1000, 8, color);
}


void vdp_print_grp1(char x, char y, char *msg)
{
	register char c;
	register uint16_t addr = vdp_base_names_grp1 + y * 32 + x;

	while ((c = *msg++ ) != '\0') {
		vdp_write(addr++, c);
	}
}
