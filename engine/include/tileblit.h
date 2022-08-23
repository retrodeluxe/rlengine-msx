/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017-2021 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#include "msx.h"

#ifndef _TILEBLIT_H_
#define _TILEBLIT_H_

void tileblit_object_show(TileObject *tileobject, uint8_t *buffer) __nonbanked;


#endif /* TILEBLIT_H */
