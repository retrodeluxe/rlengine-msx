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

#ifndef _MSX_H_
#define _MSX_H_

#include <stdbool.h>
#include <stdint.h>

#define HASH_LITERAL #
#define HASH() HASH_LITERAL

/* system variables are only available in main ROM */
#define VDP_DR 0x0006
#define VDP_DW 0x0007
#define SYS_INFO1 0x002B
#define SYS_INFO2 0x002C
#define SYS_INFO3 0x002D
#define SYS_CLIKSW 0xF3DB
#define SYS_5TH_SPRITE 0xF3E7

#define BIOS_EXTROM 0x015F
#define BIOS_DISSCR 0x0041
#define BIOS_ENASCR 0x0044
#define BIOS_CHGMOD 0x005F
#define BIOS_WRTPSG 0x0093
#define BIOS_RDPSG 0x0096
#define BIOS_INIPLT 0x0141
#define BIOS_RSTPLT 0x0145
#define BIOS_SETPLT 0x014D
#define BIOS_CHGCPU 0x0180
#define BIOS_GTPAD  0x00DB
#define BIOS_SNSMAT 0x0141
#define BIOS_CHGET  0x009f
#define BIOS_GTTRIG 0x00d8
#define BIOS_GTSTCK 0x00d5

#ifdef MSXDOS
#define call_bios(ADDR) \
        ld iy,(0xfcc0) \
        ld ix,HASH()ADDR \
        call 0x001c
#define lda_main_rom(ADDR) \
        ld a,(0xfcc1) \
        ld hl,HASH()ADDR \
        call 0x000c
#else
#define call_bios(ADDR) call (ADDR)
#define lda_main_rom(ADDR) ld a,(ADDR)
#endif

/**
 * Disable interrupts
 */
#define sys_irq_disable() __asm di __endasm

/**
 * Enable interrupts
 */
#define sys_irq_enable() __asm ei __endasm

/**
 * Wait for VSYNC
 */
#define sys_wait_vsync() __asm halt __endasm

/**
 * Declare an unused argument to avoid compiler warnings
 */
#define unused(x) x

/**
 * Defines Engine Result Codes
 */
typedef enum {
    /** Operation sucessful */
    RLE_OK,
    /** Object is already allocated */
    RLE_ALREADY_ALLOCATED,
    /** Could not allocate Object in VRAM */
    RLE_COULD_NOT_ALLOCATE_VRAM
} rle_result;

/* avoids SDCC builtins to break sphinx
 * documentation generation
 */
#ifdef HAWKMOTH
#define __nonbanked
#define __naked
#endif

#endif /* _MSX_H_ */
