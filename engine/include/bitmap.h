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

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdbool.h>
#include <stdint.h>

uint8_t bitmap_get(uint8_t *bitmap, uint8_t index);
void bitmap_set(uint8_t *bitmap, uint8_t index);
void bitmap_reset(uint8_t *bitmap, uint8_t index);
bool bitmap_find_gap(uint8_t *bitmap, uint8_t gap_size, uint8_t bitmap_size,
                     uint8_t *pos);
void bitmap_dump(uint8_t *bitmap, uint8_t bitmap_size);

#endif
