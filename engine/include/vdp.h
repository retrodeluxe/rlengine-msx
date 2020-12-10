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
#include <stdint.h>
#include <stdbool.h>

#ifndef _VDP_H_
#define _VDP_H_

typedef enum {
  MODE_TXT,
  MODE_GRP1,
  MODE_GRP2,
  MODE_MULT
} VdpMode;

typedef enum {
  SPR_SIZE_8,
  SPR_SIZE_16
} VdpSpriteSize;

typedef enum {
  SPR_ZOOM_OFF,
  SPR_ZOOM_ON
} VdpSpriteZoom;

typedef enum {
  COLOR_TRANSPARENT,
  COLOR_BLACK,
  COLOR_GREEN,
  COLOR_LIGHTGREEN,
  COLOR_DIMBLUE,
  COLOR_BLUE,
  COLOR_DIMRED,
  COLOR_LIGHTBLUE,
  COLOR_RED,
  COLOR_LIGHTRED,
  COLOR_YELLOW,
  COLOR_LIGHTYELLOW,
  COLOR_DIMGREEN,
  COLOR_MAGENTA,
  COLOR_GREY,
  COLOR_WHITE
} VdpColor;

#define VRAM_BASE_NAME 0x1800
#define VRAM_BASE_COLR 0x2000
#define VRAM_BASE_PTRN 0x0000
#define VRAM_BASE_SATR 0x1b00
#define VRAM_BASE_SPAT 0x3800

#define MAX_SPR_ATTR 32
#define MAX_SPR_PTRN 255

typedef struct VdpSpriteAttr VdpSpriteAttr;
struct VdpSpriteAttr {
  uint8_t y;
  uint8_t x;
  uint8_t pattern;
  uint8_t color;
};

#define vdp_poke_names(OFFSET, PATTERN)                                        \
  vdp_write(VRAM_BASE_NAME + (OFFSET), PATTERN)

extern void vdp_clear(VdpColor color);
extern void vdp_puts(char x, char y, char *msg);
extern void vdp_screen_disable(void);
extern void vdp_screen_enable(void);
extern void vdp_set_mode(VdpMode mode);
extern void vdp_set_color(VdpColor ink, VdpColor border);
extern void vdp_write(uint16_t address, uint8_t value) __nonbanked;
extern void vdp_memset(uint16_t vaddress, uint16_t size,
                       uint8_t value) __nonbanked;
extern void vdp_memcpy(uint16_t vaddress, uint8_t *buffer,
                       uint16_t size) __nonbanked;
extern void vdp_memcpy_vda(uint8_t *buffer) __nonbanked;
extern void vdp_init_hw_sprites(VdpSpriteSize spritesize, VdpSpriteZoom zoom);
extern void vdp_fastcopy_nametable(uint8_t *buffer) __nonbanked;
extern void vdp_rle_inflate(uint16_t vaddress, uint8_t *buffer, uint16_t size);

#endif
