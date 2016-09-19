/*
 * RetroDeLuxe Engine MSX1
 *
 * Copyright (C) 2013 Enric Geijo
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

static char t_spritesize;

// XXX: some of this calls use bios, which sometimes may not be a good idea

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
        push ix
        push iy
        ld      a,4(ix)
        ld      (0xfcaf),a
        ld      ix,#0x5f
        ld      iy,(0xfcc0)
        call    0x1c
        pop iy
        pop ix
        __endasm;
}

void vdp_set_color(char ink, char border)
{
        ink;
        border;

        __asm
        ld A,4(ix)
        ld B,5(ix)
        sla A
        sla A
        sla A
        sla A
        add A,B
        di
        out	(#0x99),A
	ld	A,#0x87
        ei
	out	(#0x99),A
        __endasm;
}


void vdp_poke(uint address, byte value)
{
        address;
        value;

        __asm
        di
        ld l,4(ix)
        ld h,5(ix)
        ld a,l
        di
        out (0x99),a
        ld a,h
        add a,#0x40
        ei
        out (0x99),a
        ld a,6(ix)
        out (0x98),a
        ei
        __endasm;
}


byte vdp_peek(uint address)
{
        address;

        __asm
        ld l,4(ix)
        ld h,5(ix)
        ld a,l
        di
        out (0x99),a
        ld a,h
        add a,#0x40
        ei
        out (0x99),a
        in a,(0x98)
        ld l,a
        __endasm;
}


void vdp_memset(uint vaddress, uint size, byte value)
{
        vaddress;
        size;
        value;

        __asm
        ld  l,4(ix)     ; vaddress
        ld  h,5(ix)
        in a,(0x99)
        ld a,l
        di
        out (0x99),a
        ld a,h
        add a,#0x40
        ei
        out (0x99),a
        ld e,6(ix)      ;length
        ld d,7(ix)
        ld a,8(ix)      ;value
        ld b,a
vdp_memset_loop:
        ld a,b
        out (0x98),A
        dec de
        ld a,d
        or e
        jr nz,vdp_memset_loop
        __endasm;
}


void vdp_copy_to_vram(byte *buffer, uint vaddress, uint length)
{
        buffer;
        vaddress;
        length;

        __asm
        ld  l,6(ix) ; vaddress
        ld  h,7(ix)
        in a,(0x99)
        di
        ld a,l
        out (0x99),a
        ld a,h
        add a,#0x40
        ei
        out (0x99),a
        ld l,4(ix) ;address
        ld h,5(ix)
        ld e,8(ix) ;length
        ld d,9(ix)
        ld c,#0x98
vdp__copy_to_vram_loop:
        outi
        dec de
        ld a,d
        or e
        jr nz,vdp__copy_to_vram_loop
        __endasm;
}

/**
 * Copy 16 bytes from RAM to VRAM
 *  - useful to set a pair of consecutive tiles
 */
void vdp_fastcopy16(byte *src_ram, uint dst_vram)
{
        src_ram;
        dst_vram;

        __asm
        ld  l,6(ix) ; dst vram
        ld  h,7(ix)
        in a,(0x99)
        ld a,l
        di
        out (0x99),a
        ld a,h
        add a,#0x40
        ei
        out (0x99),a
        ld l,4(ix)  ; src ram
        ld h,5(ix)
        ld c,#0x98
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
        outi
        outi
        outi
        __endasm;
}

void vdp_copy_from_vram(uint vaddress, byte *buffer, uint length)
{
        vaddress;
        buffer;
        length;

        __asm
        ld c,4(ix) ; vaddress
        ld b,5(ix)
        ld l,6(ix) ;address
        ld h,7(ix)
        ld e,8(ix) ;length
        ld d,9(ix)
        in a,(0x99)
        ld a,c
        di
        out (0x99),a
        ld a,b
        add a,#0x40
        ei
        out (0x99),a
        in a,(0x98)
        ld c,#0x98
vdp__copy_from_vram_loop:
        ini
        dec de
        ld a,d
        or e
        jr nz,vdp__copy_from_vram_loop
        __endasm;
}


void vdp_set_hw_sprite(char spi, struct vdp_hw_sprite *spr)
{
        vdp_copy_to_vram((byte*)spr, vdp_base_spatr_grp1+(spi<<2),sizeof(struct vdp_hw_sprite));
}

void vdp_set_hw_sprite_di(byte *spr, byte spi)
{
        spr;
        spi;

        __asm
        xor a
        ld b,a
        ld c,6(ix) ; spi
        sla c
        sla c
        ld  hl,#vdp_base_spatr_grp1
        add hl,bc
        ld a,l
        out (0x99),a
        ld a,h
        add a,#0x40
        out (0x99),a
        ld l,4(ix) ; buffer address
        ld h,5(ix)
        ld c,#0x98
        outi
        outi
        outi
        outi
        __endasm;
}

void vdp_init_hw_sprites(char spritesize, char zoom)
{
        t_spritesize = spritesize;
        zoom;
        __asm
        in a,(0x99)
        ld b,#0x00
        ld a,4(ix)
        and #0x0f
        cp #0x08
        jr z,$1
        set 1,b         ; --- if 16x16 sprites => set bit 1
$1:
        ld a,5(ix)
        cp #0x00
        jr z,$2
        set 0,b         ; --- if zoom sprites => set bit 0
$2:
        ld hl,#0xf3e0   ; --- read vdp(1) from mem
        ld a,(hl)
        and #0xfc
        or b
        ld (hl),a
        di
        out (0x99),a
        ld a,#0x81
        ei
        out (0x99),a
        __endasm;
}

void vdp_fastcopy_nametable(byte *buffer)
{
        buffer;
        __asm
        ld   hl,#vdp_base_names_grp1
        ld   a,l
        di
        out  (0x99),a
        ld   a,h
        add  a,#0x40
        ei
        out  (0x99),a
        ld   b,#96    ; 96*8 = 768 blocks
        ld   l,4(ix)  ; buffer address
        ld   h,5(ix)
        ld   c,#0x98
vdp__fastcopy_nametable_loop:
        ld   d,b
        outi
        outi
        outi
        outi
        outi
        outi
        outi
        ld   b,d
        outi
        jr   nz,vdp__fastcopy_nametable_loop
        __endasm;
}

void vdp_fastcopy_nametable_di(byte *buffer)
{
        buffer;

        __asm
        ld   hl,#vdp_base_names_grp1
        ld   a,l
        out  (0x99),a
        ld   a,h
        add  a,#0x40
        out  (0x99),a
        ld   b,#96    ; 96*8 = 768 blocks
        ld   l,4(ix)  ; buffer address
        ld   h,5(ix)
        ld   c,#0x98
vdp__fastcopy_nametable_di_loop:
        ld   d,b
        outi
        outi
        outi
        outi
        outi
        outi
        outi
        ld   b,d
        outi
        jr   nz, vdp__fastcopy_nametable_di_loop
        __endasm;
}

/*
 * Set all names to zero, then clear colors of char zero for
 *  all pattern banks
 */
void vdp_clear_grp1(byte color)
{
        vdp_memset(vdp_base_names_grp1, 256 * 3, 0);
        vdp_memset(vdp_base_color_grp1, 8, color);
        vdp_memset(vdp_base_color_grp1 + 0x800, 8, color);
        vdp_memset(vdp_base_color_grp1 + 0x1000, 8, color);
}


void vdp_print_grp1(char x, char y, char *msg)
{
        register char c;
        register uint addr = vdp_base_names_grp1 + y * 32 + x;

        while ((c = *msg++ ) != '\0') {
                vdp_poke(addr++, c);
        }
}


