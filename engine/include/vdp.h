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

/**
 * VDP Mode
 */
typedef enum {
  /**
   * Text Mode (Screen 0)
   */
  MODE_TXT,

  /**
   * Graphic Mode 1 (Screen 1)
   */
  MODE_GRP1,

  /**
   * Graphic Mode 2 (Screen 2)
   */
  MODE_GRP2,

  /**
   * Multicolor Mode (Screen 3)
   */
  MODE_MULT,

  /**
   * Graphics Mode 3 (Screen 4, MSX2)
   */
   MODE_GRP3,

  /**
   * Graphics Mode 4 (Screen 5, MSX2)
   */
   MODE_GRP4,

  /**
   * Graphics Mode 5 (Screen 6, MSX2)
   */
   MODE_GRP5,

  /**
   * Graphics Mode 6 (Screen 7, MSX2)
   */
   MODE_GRP6

} VdpMode;

/**
 * Sprite Size
 */
typedef enum {
  /**
   * Use 8x8 Hardware Sprites
   */
  SPR_SIZE_8,

  /**
   * Use 16x16 Hardware Sprites
   */
  SPR_SIZE_16
} VdpSpriteSize;

/**
 * Sprite zoom
 */
typedef enum {
  /**
   * Disable sprites zoom
   */
  SPR_ZOOM_OFF,
  /**
   * Enable sprite zoom
   */
  SPR_ZOOM_ON
} VdpSpriteZoom;

/**
 * Vdp Palette
 */
typedef enum {
  /** */
  COLOR_TRANSPARENT,
  /** */
  COLOR_BLACK,
  /** */
  COLOR_GREEN,
  /** */
  COLOR_LIGHTGREEN,
  /** */
  COLOR_DIMBLUE,
  /** */
  COLOR_BLUE,
  /** */
  COLOR_DIMRED,
  /** */
  COLOR_LIGHTBLUE,
  /** */
  COLOR_RED,
  /** */
  COLOR_LIGHTRED,
  /** */
  COLOR_YELLOW,
  /** */
  COLOR_LIGHTYELLOW,
  /** */
  COLOR_DIMGREEN,
  /** */
  COLOR_MAGENTA,
  /** */
  COLOR_GREY,
  /** */
  COLOR_WHITE,
  MAX_COLORS
} VdpColor;

#define VRAM_BASE_NAME 0x1800
#define VRAM_BASE_COLR 0x2000
#define VRAM_BASE_PTRN 0x0000
#define VRAM_BASE_SATR 0x1b00
#define VRAM_BASE_SPAT 0x3800

#define VRAM_BASE_MULT_NAME 0x0800
#define VRAM_BASE_MULT_PTRN 0x0000

#define VRAM_BASE_GRP3_SATR 0x1e00 // SATR moves down to accomodate colors
#define VRAM_BASE_GRP3_SCOL 0x1c00 // 512 bytes before SATR

#define VRAM_BASE_GRP4_PTRN 0x0000

#define MODE_GRP4_SCR_WIDTH 256
#define MODE_GRP4_PIX_PER_BYTE 2
#define MODE_GRP6_SCR_WIDTH 512
#define MODE_GRP6_PIX_PER_BYTE 2

/* v99xx access registers */
#define V99xx_SET_VRAM_PAGE   0x0E
#define V99xx_SET_STATUS_REG  0x0F
#define V99xx_SET_PALETTE_REG 0x10
#define V99xx_SET_CONTROL_REG 0x1
#define V99xx_MODE_REG_8 0x8
#define V99xx_MODE_REG_9 0x9

#define MAX_SPR_ATTR 32
#define MAX_SPR_PTRN 255
#define SPR_OFF 208

typedef struct VdpSpriteAttr VdpSpriteAttr;
struct VdpSpriteAttr {
  uint8_t y;
  uint8_t x;
  uint8_t pattern;
  uint8_t color;
};

/**
 * Defines an 9-bit RGB color
 */
typedef struct VdpRGBColor VdpRGBColor;
/**
 * Contains an 9-bit RGB color
 */
struct VdpRGBColor {
  /** red component (0 to 7) */
  uint8_t r;
  /** green component (0 to 7) */
  uint8_t g;
  /** blue component (0 to 7) */
  uint8_t b;
};

/**
 * Vdp Blit Command Types
 */
typedef enum {
  STOP,
  POINT = 4,
  PSET,
  SRCH,
  LINE,
  LMMV,
  LMMM,
  LMCM,
  LMMC,
  HMMV,
  HMMM,
  YMMM,
  HMMC
} VdpCommandType;

/**
 *  Vdp Logical modifiers for Logical operations
 */
 typedef enum {
   IMP,
   AND,
   OR,
   XOR,
   NOT,
   TIMP = 8,
   TAND,
   TOR,
   TXOR,
   TNOT
} VdpCommandLogicType;

/**
 * Defines an vdp command
 */
typedef struct VdpCommand VdpCommand;
/**
 * Contains a vdp command definition
 */
struct VdpCommand {
  /**
   * Source screen coordinate x
   */
  uint16_t sx;
  /**
   * Source screen coordinate y
   */
  uint16_t sy;
  /**
   * Destination screen coordinate x
   */
  uint16_t dx;
  /**
   * Destination screen coordinate y
   */
  uint16_t dy;
  /**
   * Number of pixels in x axis
   */
  uint16_t nx;
  /**
   * Number of pixels in y axis
   */
  uint16_t ny;
  /**
   * Color
   */
  uint8_t color;
  /**
   * Transfer destination and direction
   */
  uint8_t destdir;
  /**
   * Actual command to be executed
   */
  uint8_t command;
};


#define vdp_poke_names(OFFSET, PATTERN)                                        \
  vdp_write(VRAM_BASE_NAME + (OFFSET), PATTERN)

extern void vdp_clear(VdpColor color);
extern void vdp_puts(char x, char y, char *msg);
extern void vdp_screen_disable(void);
extern void vdp_screen_enable(void);
extern void vdp_set_mode(VdpMode mode);
extern VdpMode vdp_get_mode(void);
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
extern uint8_t vdp_5th_sprite() __nonbanked;

/* MSX2 and above */
extern void vdp_set_palette(VdpRGBColor *palette);
extern void vdp_set_palette_color(uint8_t index, VdpRGBColor *color);
extern void vdp_write_reg(uint8_t reg, uint8_t val) __nonbanked;
extern void vdp_set_vram_page(uint8_t page) __nonbanked;
extern void vdp_exec(VdpCommand *cmd) __nonbanked;
extern void vdp_sprite_disable(void) __nonbanked;
extern void vdp_set_192_lines(void) __nonbanked;
#endif
