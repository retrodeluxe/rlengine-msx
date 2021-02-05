/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2020 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#ifndef _BLIT_H_
#define _BLIT_H_

/**
 * Defines a BlitSet
 */
typedef struct BlitSet BlitSet;
/**
 * Contents of a BlitSet
 */
struct BlitSet {
  /**
   * BlitSet width in pixels
   */
  uint16_t w;
  /**
   * BlitSet height in pixels
   */
  uint8_t h;
  /**
   * Pointer to the BlitSet bitmap data
   */
  uint8_t *bitmap;
  /**
   * page of the BlitSet once allocated into VRAM
   */
  uint8_t page;
  /**
   * x Position of the BlitSet once allocated into VRAM
   */
  uint16_t xpos;
    /**
   * y Position of the BlitSet once allocated into VRAM
   */
  uint16_t ypos;
  /**
   * True if the BlitSet is allocated in VRAM
   */
  bool allocated;
  /**
   * True if the BlitSet data is not compressed
   */
  bool raw;
  /**
   * Animation frame width in pixels
   */
  uint8_t frame_w;
  /**
   * Animation frame height in pixels
   */
  uint8_t frame_h;
  /**
   * Number of animation frames per state (regular)
   */
  uint8_t frames;
  /**
   * Number of animation states within the set
   */
  uint8_t states;
};

/**
 * Defines a BlitObject
 */
typedef struct BlitObject BlitObject;
/**
 * Contens of a BlitObject
 */
struct BlitObject {
  /**
   * Screen X position in pixel coordinates
   */
  uint16_t x;
  /**
   * Screen Y position in pixel coordinates
   */
  uint16_t y;
  /**
   * Previous screen X position in pixel coordinates
   */
  uint16_t prev_x;
  /**
   * Previous screen Y position in pixel coordinates
   */
  uint16_t prev_y;
  /**
   * mask X position in pixel coordinates
   */
  uint16_t mask_x;
  /**
   * mask Y position in pixel coordinates
   */
  uint16_t mask_y;
  /**
   * mask page
   */
  uint8_t mask_page;
  /**
   * Current animation state
   */
  uint8_t state;
  /**
   * Current animation frame
   */
  uint8_t frame;
  /**
   * Animation counter
   */
  uint8_t anim_ctr;
  /**
   * Current animation state for composite objects
   */
  uint8_t state2;
  /**
   * Current animation frame for composite objects
   */
  uint8_t frame2;
  /**
   * X offset of blitset2
   */
  int8_t offsetx2;
  /**
   * y offset of blitset2
   */
  int8_t offsety2;
  /**
   * BlitSet data
   */
  BlitSet *blitset;
  /**
   * BlitSet data
   */
  BlitSet *blitset2;
};

 /**
 * Initialize a Static BlitSet
 *
 * :param TS: a BlitSet object
 * :param DATA: name of data asset
 */
#define INIT_BLIT_SET(TS, DATA, W, H)                                           \
  (TS).w = DATA##_bitmap_w;                                                     \
  (TS).h = DATA##_bitmap_h;                                                     \
  (TS).bitmap = DATA##_bitmap;                                                  \
  (TS).allocated = false;                                                       \
  (TS).frame_w = W;                                                             \
  (TS).frame_h = H;                                                             \
  (TS).raw = false;

 /**
 * Initialize a Dynamic BlitSet
 *
 * :param TS: a BlitSet object
 * :param DATA: name of data asset
 * :param W: frame width of the blitset in pixels
 * :param H: frame heigth of the blitset in pixels
 * :param F: number of frames per state
 * :param S: number of states
 */
#define INIT_DYNAMIC_BLIT_SET(TS, DATA, W, H, F, S)                             \
  (TS).w = DATA##_bitmap_w;                                                     \
  (TS).h = DATA##_bitmap_h;                                                     \
  (TS).bitmap = DATA##_bitmap;                                                  \
  (TS).allocated = false;                                                       \
  (TS).frame_w = W;                                                             \
  (TS).frame_h = H;                                                             \
  (TS).frames = F;                                                              \
  (TS).states = S;                                                              \
  (TS).raw = false;

extern void blit_init();
extern rle_result blit_set_valloc(BlitSet *blitset);
extern void blit_set_vfree(BlitSet *blitset);
extern void blit_set_to_vram(BlitSet *blitset, uint8_t page,
                    uint16_t xpos, uint16_t ypos) __nonbanked;
extern void blit_object_show(BlitObject *blitobject) __nonbanked;
extern void blit_object_hide(BlitObject *blitobject) __nonbanked;
extern void blit_object_update(BlitObject *blitobject) __nonbanked;
extern void blit_map_tilebuffer(uint8_t *buffer, BlitSet *bs, uint8_t page) __nonbanked;

#endif /* _BLIT_H_ */
