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

#ifndef _MSX_H_MAP
#define _MSX_H_MAP

#define map_inflate_screen(MAP, BUF, X, Y)                                     \
  __map_inflate_screen(MAP##_cmpr_dict, MAP, BUF, MAP##_w, X, Y);

void map_inflate(const uint8_t *dict, const uint8_t *in, uint8_t *out,
                 uint16_t data_size, uint8_t w) __nonbanked;
void __map_inflate_screen(const uint8_t *dict, const uint16_t *in, uint8_t *out,
                          uint8_t w, uint8_t vpx, uint8_t vpy) __nonbanked;

#endif
