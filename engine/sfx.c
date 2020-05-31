/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017-2020 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

/*
 *      Based on ayFX REPLAYER v1.31
 *	code: SapphiRe
 *	source: http://z80st.auic.es
 */

 /* This module has a dependency on PT3 player */

/* ayFX afb file format
	+0 - number of effects in the file
	+1 - effects table
	     - 2 byte pointer to effect within the file

	each effect contains N frames of variable length
	+0 - Flags
	     bits 0..3 Volume
	     bit  4    Disable Tone
	     bit  5    Change Tone
	     bit  6    Change Noise
	     bit  7    Disable Noise

	if bit 6 = 1
	+ 1-2 : Tone period

	if bit 5 = 1
	+ 1 : Noise period

	if bit 6 =1 and bit 5 =1
	+ 1-2 : Tone period
	+ 3   : Noise period

	bit 4 and bit 7 are used to clear Tone and Noise

	End of effect is marked as 0xD0 0x20
*/


#include "pt3.h"

#pragma CODE_PAGE 2

uint8_t *sfx_bank;
uint8_t *sfx_pointer;
uint8_t sfx_mode;
uint8_t sfx_priority;
uint8_t sfx_noise;
uint8_t sfx_volume;
uint8_t sfx_channel;
uint16_t sfx_tone;
uint8_t *sfx_volume_table;

void sfx_setup(uint8_t *bank)
{
	sfx_bank = bank;
	sfx_mode = 0;
	sfx_channel = 3;
	sfx_priority = 255;
}

void sfx_play_effect(uint8_t effect, uint8_t priority) __nonbanked
{
	effect;
	priority;

	__asm
	ld	b,4(ix)
	ld	c,5(ix)
	ld	hl,(#_sfx_bank)
	ld	a,(hl)
	or	a
	jr	z, check_priority
	ld	a,b
	cp	(hl)
	ld	a,#2
	jr	nc, init_end
check_priority:
	ld	a,b
	ld	a,(#_sfx_priority)
	cp	c
	ld	a,#1
	jr	c, init_end
	ld	a,c
	and	#0x0F
	ld	(#_sfx_priority), a

	ld	de,(#_sfx_bank)
	inc	de
	ld	l,b
	ld	h,#0
	add	hl,hl
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	add	hl,de
	ld	(#_sfx_pointer), hl
	xor	a
init_end:
	jp	end
init_nosound:
	ld	a,#255
	ld	(#_sfx_priority),a
end:
	__endasm;
}

void sfx_play(void) __naked __nonbanked
{
	__asm
	ld	a,(#_sfx_priority)
	or	a
	ret	m
	ld	a,(#_sfx_mode)
	and	#1
	jr	z, get_control_byte
	ld	hl, #_sfx_channel
	dec	(hl)
	jr	nz, get_control_byte
	ld	(hl), #3
get_control_byte:
	ld	hl,(#_sfx_pointer)
	ld	c,(hl)
	inc	hl
	bit	5,c
	jr	z, check_new_tone
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	(#_sfx_tone),de
check_new_tone:
	bit	6,c
	jr	z, set_pointer
	ld	a,(hl)
	inc	hl
	cp	#0x20
	jr	z, sfx_end
	ld	(#_sfx_noise),a
set_pointer:
	ld	(#_sfx_pointer),hl
	ld	a,c
	and 	#0x0F

	ld	(#_sfx_volume),a
	ret	z

	// copy sfx into AY registers
	bit 	7,c
	jr	nz, set_masks
	ld	a,(#_sfx_noise)
	ld	(#_AYREGS + 6),a
set_masks:
	ld	a,c
	and 	#0x90
	cp	#0x90
	ret	z
	rrca
	rrca
	ld	d,#0xDB
	ld	hl, #_sfx_channel
	ld	b,(hl)
	djnz	check2
play_c:
	call	set_mixer
	ld	(#_AYREGS + 10), a
	bit	2,c
	ret	nz
	ld	(#_AYREGS + 4), hl
	ret
check2:
	rrc	d
	rrca
	djnz	check3
play_b:
	call	set_mixer
	ld	(#_AYREGS + 9), a
	bit	1,c
	ret	nz
	ld	(#_AYREGS + 2), hl
	ret
check3:
	rrc	d
	rrca
play_a:
	call	set_mixer
	ld	(#_AYREGS + 8), a
	bit	0,c
	ret	nz
	ld	(#_AYREGS + 0), hl
	ret
set_mixer:
	ld	c,a
	ld	a,(#_AYREGS + 7)
	and	d
	or	c
	ld	(#_AYREGS + 7),a
	ld	a,(#_sfx_volume)
	ld	hl,(#_sfx_tone)
	ret
sfx_end:
	ld	a,#255
	ld	(#_sfx_priority), a

	// mute effect
	ld 	hl,#_AYREGS
	ld 	a,(#_sfx_channel)
	inc 	a
	and 	#3
	add 	a,#8
	add 	a,l
	ld 	l,a
	adc 	a,h
	sub 	l
	ld 	h,a
	ld 	(hl),#0
	ret
	__endasm;
}
