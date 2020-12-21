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

#include "bitmap.h"
#include "log.h"

inline static uint8_t get(uint8_t a, uint8_t pos) __nonbanked {
  return (a >> pos) & 1;
}

inline static void set(uint8_t *a, uint8_t pos) __nonbanked { *a |= 1 << pos; }

inline static void reset(uint8_t *a, uint8_t pos) __nonbanked {
  *a &= ~(1 << pos);
}

uint8_t bitmap_get(uint8_t *bitmap, uint8_t pos) __nonbanked {
  return get(bitmap[pos / 8], pos % 8) != 0;
}

void bitmap_set(uint8_t *bitmap, uint8_t pos) __nonbanked {
  set(&bitmap[pos / 8], pos % 8);
}

void bitmap_reset(uint8_t *bitmap, uint8_t pos) __nonbanked {
  reset(&bitmap[pos / 8], pos % 8);
}

/**
 * bitmap_find_gap:
 *      find a sequence of n 1s in the bitmap
 */
bool bitmap_find_gap(uint8_t *bitmap, uint8_t gap_size, uint8_t bitmap_size,
                     uint8_t *pos) __nonbanked {
  uint8_t i, nfree = 0;
  for (i = 0, bitmap_size *= 8; i < bitmap_size; i++) {
    nfree = nfree * bitmap_get(bitmap, i) + bitmap_get(bitmap, i);
    if (nfree == gap_size) {
      *pos = i - gap_size + 1;
      return true;
    }
  }
  return false;
}

// void bitmap_dump(uint8_t *bitmap, uint8_t bitmap_size) {
//   uint8_t i;
//   log_e("bitmap dump\n");
//   for (i = 0; i < bitmap_size * 8; i++) {
//     log_e("%d - %d\n", i, bitmap_get(bitmap, i));
//   }
//   log_e("\n");
//}
