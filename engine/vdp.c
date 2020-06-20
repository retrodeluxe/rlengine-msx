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
uint8_t rle_prev;
uint8_t rle_prev_run;

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


/**
 *
 */
static void vdp_write_internal() __naked
{
	__asm
	ex	af,af'
	ld 	a,e
	out 	(0x99),a
	ld	a,d
	add 	a,#0x40
	out 	(0x99),a
	ex	af,af'
	out 	(0x98),a
	inc 	de
	ret
	__endasm;
}

void vdp_rle_inflate(uint16_t vaddress, uint8_t *buffer, uint16_t size)
{
	rle_prev = 0;
	rle_prev_run = 255;

	__asm
	di
	ld 	e, 4 (ix)
	ld 	d, 5 (ix)
	ld	c, 8 (ix)
	ld	b, 9 (ix)
	ld	l, 6 (ix)
	ld	h, 7 (ix)
loop1:
	push 	bc
	ld	a,(hl)
	call	_vdp_write_internal
	ld	a,(#_rle_prev_run)
	ld	b,(hl)
	inc 	hl
	or	a
	jr	nz, prev_run
	ld	a,(#_rle_prev)
	cp 	b
	jr	z, run_mode
prev_run:
	ld	a,#0
	ld	(#_rle_prev_run),a
	ld	a,b
	ld	(#_rle_prev),a
	pop 	bc
	dec 	bc
	ld	a,b
	or	c
	jr	nz, loop1
	jp	end_rle
run_mode:
	pop 	bc
	push 	hl
	ex	af, af'
	ld	a,#255
	ld	(#_rle_prev_run),a
	ld	a,(hl)
	or	a
	jr	z,skip
	ld	h,b
	ld	l,c
	ld	b,a
run_loop:
	dec 	hl
	ld	a,h
	or 	l
	jr	nz, cont
	pop 	hl
	jr	end_rle
cont:
	ex	af,af'
	call	_vdp_write_internal
	ex	af,af'
	djnz	run_loop
	ld	b,h
	ld	c,l
skip:
	dec 	bc
	pop 	hl
	inc 	hl
	ld	a,b
	or	c
	jr	nz, loop1
end_rle:
	ei
	__endasm;
}

/**
 * Fills name table with zeros and define zero char color in all banks
 */
void vdp_clear(uint8_t color)
{
	vdp_memset(vdp_base_names_grp1, 256 * 3, 0);
	vdp_memset(vdp_base_color_grp1, 8, color);
	vdp_memset(vdp_base_color_grp1 + 0x800, 8, color);
	vdp_memset(vdp_base_color_grp1 + 0x1000, 8, color);
}

/**
 * Writes a string to vram assuming scr2 and default character set is defined
 */
void vdp_puts(char x, char y, char *msg)
{
	register char c;
	register uint16_t addr = vdp_base_names_grp1 + y * 32 + x;

	while ((c = *msg++ ) != 0) {
		vdp_write(addr++, c);
	}
}
