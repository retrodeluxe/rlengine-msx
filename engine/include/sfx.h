/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017-2020 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#ifndef _SFX_H_
#define _SFX_H

void sfx_setup(uint8_t *bank);
void sfx_play_effect(uint8_t effect, uint8_t priority) __nonbanked;
void sfx_play(void) __nonbanked;

#endif
