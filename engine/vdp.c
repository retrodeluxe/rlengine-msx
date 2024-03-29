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
#include "log.h"
#include "vdp.h"

#pragma CODE_PAGE 2

uint8_t rle_prev;
uint8_t rle_prev_run;
uint8_t vdp_mode;

/**
 * Disable Screen
 */
void vdp_screen_disable(void)
{
	__asm
	call_bios(BIOS_DISSCR)
	__endasm;
}

/**
 * Enable Screen
 */
void vdp_screen_enable(void)
{
	__asm
	call_bios(BIOS_ENASCR)
	__endasm;
}

/**
 * Set VDP mode
 *
 * :param mode: see :c:enum:`VdpMode`
 *
 */
void vdp_set_mode(VdpMode mode)
{
  vdp_mode = mode;

	__asm
	ld	a,4(ix)
	call_bios(BIOS_CHGMOD)
	__endasm;
}

/**
 * Gets currently set mode
 *
 * :returns: mode see :c:enum:`VdpMode`
 */
VdpMode vdp_get_mode(void)
{
  return vdp_mode;
}


/**
 * Checks 5th sprite collision flag
 *
 *   .. warning::
 *
 *      This function must be called right after :c:func:`sys_wait_vsync`
 *
 * :returns: index of the 5th sprite attribute, or zero
 */
uint8_t vdp_5th_sprite() __naked
{
  __asm
  ld a,(SYS_5TH_SPRITE)
  bit #6,a
  jr z,no_5th_spr
  and #0x1F
  ld l,a
  ret
no_5th_spr:
  xor a
  ld l,a
  ret
  __endasm;
}

/**
 * Sets foreground and border screen color
 *
 * :param ink: see :c:enum:`VdpColor`
 * :param border: see :c:enum:`VdpColor`
 *
 */
void vdp_set_color(VdpColor ink, VdpColor border)
{
	unused(ink);
	unused(border);

	__asm
	lda_main_rom(VDP_DW)
	inc 	a
	ld	c,a
	ld	a,4(ix)
	ld	b,5(ix)
	sla	a
	sla	a
	sla	a
	sla	a
	add	a,b
	di
	out	(c),a
	ld	a,#0x87
	ei
	out	(c),a
	__endasm;
}


#ifdef MSX2
/**
 * Sets a complete palette and saves it to VRAM (MSX2)
 *
 * :param palette: a RGB color array with 16 elements
 *
 */
void vdp_set_palette(VdpRGBColor *palette) {
  uint8_t i;
  for (i = 0; i < MAX_COLORS; i++)
    vdp_set_palette_color(i, &palette[i]);
}

/**
 * Sets a palette color and saves it to VRAM (MSX2)
 *
 * :param index: palette index to set (0 to 15)
 * :param color: RGB color definition
 */
void vdp_set_palette_color(uint8_t index, VdpRGBColor *color)
{
  unused(index);
  unused(color);

  __asm
  ld d,4(ix)
  ld h,6(ix)
  ld l,5(ix)
  ld a,(hl)
  sla a
  sla a
  sla a
  sla a
  inc hl
  ld e,(hl)
  inc hl
  or (hl)
  ld ix, #BIOS_SETPLT
  call_bios(BIOS_EXTROM)
  __endasm;
}

void vdp_write_reg(uint8_t reg, uint8_t val) __nonbanked
{
  unused(reg);
  unused(val);

  __asm
  ld d,5(ix)
  ld e,4(ix)
  lda_main_rom(VDP_DW)
  inc a
  ld c,a
  out (c), d
  ld a,e
  or #0x80
  out (c), a
  __endasm;
}

void vdp_sprite_disable(void) __nonbanked
{
  vdp_write_reg(V99xx_MODE_REG_8, 10);
}

void vdp_set_192_lines(void) __nonbanked
{
  vdp_write_reg(V99xx_MODE_REG_9, 0);
}

void vdp_set_sprite_page2(void) __nonbanked
{
  vdp_write_reg(V99xx_MODE_REG_5, 0x6F);
  vdp_write_reg(V99xx_MODE_REG_11, 0x1);
  vdp_write_reg(V99xx_MODE_REG_6, 0x17);
}

void vdp_display_adjust(int8_t dx, int8_t dy) __nonbanked
{
	uint8_t adjust;

	adjust = ((dy & 7) << 4) + (dx & 7);
	if (dy < 0) adjust &= 128;
	if (dx < 0) adjust &= 8;

	vdp_write_reg(V99xx_DISP_ADJUST, adjust);
}

/**
 * Sets current VRAM 16Kb page to access up to 128Kb
 *
 * :param page: vram page to be selected (0 to 7)
 */
void vdp_set_vram_page(uint8_t page) __nonbanked
{
  vdp_write_reg(V99xx_SET_VRAM_PAGE, page & 0x7);
}


void vdp_set_vert_offset(uint8_t offset) __nonbanked
{
  vdp_write_reg(V99xx_MODE_REG_23, offset);
}

/**
 * Execute a VDP command
 *
 * :param cmd: see :c:typedef:`VdpCommand`
 */
void vdp_exec(VdpCommand *cmd) __nonbanked
{
   /* from http://map.grauw.nl/articles/vdp_tut.php#vram */
   unused(cmd);

  __asm
  ld l,4(ix)
  ld h,5(ix)
  ld a,#32
  di
  out (#0x99),a
  ld a,#(17 + 128)
  out (0x99),a
  ld c,#0x9B
vdp_ready:
  ld a,#2
  di
  out (0x99),a     ; select s#2
  ld a,#(15 + 128)
  out (0x99),a
  in a,(0x99)
  rra
  ld a,#0          ; back to s#0, enable ints
  out (0x99),a
  ld a,#(15 + 128)
  ei
  out (0x99),a     ; loop if vdp not ready (CE)
  jp c,vdp_ready
  outi            ; 15x OUTI
  outi            ; (faster than OTIR)
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  outi
  __endasm;
}

#endif

/**
 * Write a single byte to VRAM
 *
 * :param address: VRAM address (0 to 16384)
 * :param value: byte to be written
 */
void vdp_write(uint16_t address, uint8_t value) __nonbanked
{
	unused(address);
	unused(value);

	__asm
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	di
	ld	l,4(ix)
	ld	h,5(ix)
	ld	a,l
	di
	out	(c),a
	ld	a,h
	add	a,#0x40
	ei
	out	(c),a
	ld	a,6(ix)
	dec	c
	out	(c),a
	__endasm;
}

/**
 * Writes value into an VRAM range
 *
 * :param vaddress: VRAM target address (0 to 16384)
 * :param size: number of bytes to write
 * :param value: byte to be written
 */
void vdp_memset(uint16_t vaddress, uint16_t size, uint8_t value) __nonbanked
{
	unused(vaddress);
	unused(size);
	unused(value);

	__asm
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	ld	l,4(ix)
	ld	h,5(ix)
	in	a,(c)
	ld	a,l
	di
	out	(c),a
	ld	a,h
	add	a,#0x40
	out	(c),a
	ld	e,6(ix)
	ld	d,7(ix)
	ld	a,8(ix)
	ld	b,e
	dec	de
	inc	d
	dec 	c
$1:
	out	(c),a
	djnz	$1
	dec	d
	jr	nz,$1
	ei
	__endasm;
}

/**
 * Copy block from RAM to VRAM
 *
 * :param vaddress: VRAM target address (0 to 16384)
 * :param buffer: pointer to RAM source
 * :param size: number of bytes to copy
 */
void vdp_memcpy(uint16_t vaddress, uint8_t *buffer, uint16_t size) __nonbanked
{
	unused(buffer);
	unused(vaddress);
	unused(size);

	__asm
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	ld	l,4(ix)
	ld	h,5(ix)
	ld	a,l
	di
	out	(c),a
	ld	a,h
	add	a,#0x40
	out	(c),a
	ld	l,6(ix)
	ld	h,7(ix)
	ld	e,8(ix)
	ld	d,9(ix)
	dec 	c
	ld	b,e
	dec	de
	inc	d
$2:
	outi
	jp	nz,$2
	dec	d
	jr	nz,$2

#ifndef BIOS_SWITCH
	ei
#endif

	__endasm;
}

/**
 * Copy buffer with built-in address information to VRAM
 *
 * This function is equivalent to BLOAD"",S in Basic
 *
 * :param buffer: pointer to RAM source
 */
void vdp_memcpy_vda(uint8_t *buffer) __nonbanked
{
	unused(buffer);

	__asm
	ld	l,4(ix)
	ld	h,5(ix)
	inc 	hl
	ld	e, (hl)
	inc 	hl
	ld	d, (hl)
	lda_main_rom(VDP_DW)
	inc 	a
	ld	c,a
	ld	a,e
	di
	out	(c),a
	ld	a,d
	add	a,#0x40
	out	(c),a
	ld	l,4(ix)
	ld	h,5(ix)
	inc 	hl
	ld	c, (hl)
	inc 	hl
	ld	b, (hl)
	inc 	hl
	ld	e, (hl)
	inc 	hl
	ld	d, (hl)
	inc 	hl
	inc 	hl
	inc 	hl
	push 	hl
	ex 	de,hl
	sbc 	hl,bc
	ex 	de,hl
	inc	de
	pop 	hl
	lda_main_rom(VDP_DW)
	ld	c,a
	ld	b,e
	dec	de
	inc	d
$7:
	outi
	jp	nz,$7
	dec	d
	jr	nz,$7
	ei
	__endasm;
}

/**
 * Initialize VDP sprites
 *
 * :param spritesize: see :c:enum:`VdpSpriteSize`
 * :param zoom: see :c:enum:`VdpSpriteZoom`
 */
void vdp_init_hw_sprites(VdpSpriteSize spritesize, VdpSpriteZoom zoom)
{
	unused(zoom);
  unused(spritesize);

	__asm
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	ld	b,#0x00
	ld	a,4(ix)
	cp	#0x00
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
	out	(c),a
	ld	a,#0x81
	ei
	out	(c),a
	__endasm;
}

/**
 * Fast copy of a RAM buffer of 768 bytes to VRAM, on the name table.
 *
 * This function provides a faster way to refresh tiles on the screen.
 *
 * :param buffer: pointer to RAM buffer of 768 bytes
 */
void vdp_fastcopy_nametable(uint8_t *buffer) __nonbanked
{
	unused(buffer);

	__asm
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	ld	hl,#VRAM_BASE_NAME
	ld	a,l
	di
	out	(c),a
	ld	a,h
	add	a,#0x40
	out	(c),a
	ld	b,#0		; 256*3 = 768 blocks
	ld	l,4(ix)  	; buffer address
	ld	h,5(ix)
	dec	c
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


static void vdp_write_internal() __naked
{
	__asm
	ex	af,af'    ;'
	lda_main_rom(VDP_DW)
	inc	a
	ld	c,a
	ld 	a,e
	out 	(c),a
	ld	a,d
	add 	a,#0x40
	out 	(c),a
	ex	af,af'    ;'
	dec	c
	out 	(c),a
	inc 	de
	ret
	__endasm;
}

/**
 * Decompress and Copy a RAM buffer into VRAM
 *
 * This function performs Run-Length-Encoding decompression, and can be used
 * to tranfer big compressed assets (pattern definitions) to VRAM
 *
 * :param vaddress: VRAM target address
 * :param buffer: pointer to source RAM buffer
 * :param size: size of decompressed buffer
 */
 // FIXME: this appears to be failing for long data?
void vdp_rle_inflate(uint16_t vaddress, uint8_t *buffer, uint16_t size)
{
	rle_prev = 0;
	rle_prev_run = 255;

  unused(vaddress);
  unused(buffer);
  unused(size);

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
	ex	af, af'   ;'
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
	ex	af,af'   ;'
	call	_vdp_write_internal
	ex	af,af'   ;'
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

#ifndef BIOS_SWITCH
	ei
#endif

	__endasm;
}

/**
 * Clears screen by filling the name table with zeros and defining a tile
 * with index zero and the provided color.
 */
void vdp_clear(VdpColor color)
{
	vdp_memset(VRAM_BASE_NAME, 256 * 3, 0);
	vdp_memset(VRAM_BASE_COLR, 8, color);
	vdp_memset(VRAM_BASE_COLR + 0x800, 8, color);
	vdp_memset(VRAM_BASE_COLR + 0x1000, 8, color);
}

/**
 * Writes a String to VRAM on mode `MODE_GRP1`
 *
 * :param x: x coordinate (0-32)
 * :param y: y coordinate (0-24)
 * :param msg: String to be written
 */
void vdp_puts(char x, char y, char *msg)
{
	register char c;
	register uint16_t addr = VRAM_BASE_NAME + y * 32 + x;

	while ((c = *msg++ ) != 0) {
		vdp_write(addr++, c);
	}
}
