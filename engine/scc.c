/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2022 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#include "msx.h"
#include "sys.h"
#include "scc.h"
#include "log.h"

static uint8_t scc_slot;

#define scc_map() ascii8_set_slot_page2(scc_slot);
#define scc_unmap() ascii8_set_rom_page2();

static void scc_detect() __nonbanked
{
  uint8_t val;

  for (scc_slot = 0; scc_slot < 255; scc_slot++) {

      scc_map();

      // make sure we are in ROM
      val = SCC_BASE;
      SCC_BASE = val + 1;

      if (SCC_BASE != val) {
        SCC_BASE = val;
        continue;
      }

      // check wave table is rw
      SCC_SET_PAGE = SCC_SELECT;

      val = SCC_WAVE_BASE;
      SCC_WAVE_BASE = val + 1;

      if (SCC_WAVE_BASE != val) {
        scc_unmap();
        return;
      }

  }

  scc_unmap();
}

void scc_init() __nonbanked
{
    scc_detect();
}

void scc_unmute(uint8_t chan) __nonbanked
{
    scc_map();

    SCC_ENABLE = ~chan;

    scc_unmap();
}

void scc_mute(uint8_t chan) __nonbanked
{
    scc_map();

    SCC_ENABLE = chan;

    scc_unmap();
}

void scc_set_wave(uint8_t chan, uint8_t *data) __nonbanked
{
    scc_map();

    sys_memcpy(&SCC_WAVE_BASE + SCC_WAVE_LEN * chan, data, 32);

    scc_unmap();
}

void scc_set_vol(uint8_t chan, uint8_t vol) __nonbanked
{
    scc_map();

    *(&SCC_VOL_BASE + chan) = vol;

    scc_unmap();
}

void scc_set_freq(uint8_t chan, uint16_t freq) __nonbanked
{
    scc_map();

    *(&SCC_FREQ_BASE + chan) = freq;

    scc_unmap();
}
