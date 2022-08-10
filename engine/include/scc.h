/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017-2022 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

// FIXME: for now this is has a dependency with megarom
#include "ascii8.h"

#define SCC_SELECT 0x3f

// can we do this with sfr?
#define SCC_BASE      *((uint8_t *)0x8000)
#define SCC_SET_PAGE  *((uint8_t *)0x9000)
#define SCC_WAVE_BASE *((uint8_t *)0x9800)
#define SCC_VOL_BASE  *((uint8_t *)0x988A)
#define SCC_FREQ_BASE *((uint8_t *)0x9880)
#define SCC_ENABLE    *((uint8_t *)0x988F)

#define SCC_CH1       1 << 0
#define SCC_CH2       1 << 1
#define SCC_CH3       1 << 2
#define SCC_CH4       1 << 3
#define SCC_CH5       1 << 5

#define SCC_WAVE_LEN 32

#define scc_map() ascii8_set_slot_page2(scc_slot);
#define scc_unmap() ascii8_set_rom_page2();

extern void scc_init();
extern void scc_enable(uint8_t chmask);
extern void scc_mute(uint8_t chmask);
extern void scc_set_wave(uint8_t chan, uint8_t *data);
extern void scc_set_vol(uint8_t chan, uint8_t vol);
extern void scc_set_freq(uint8_t chan, uint16_t freq);
