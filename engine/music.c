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

/*
 * Trilo Tracker PSG Replayer Copyright 2016 Richard Cornelisse
 *
 * Ported to SDCC for RDL Engine by Enric Martin Geijo 2018
 */

#include "psg.h"
#include "music.h"
#include "sys.h"

static uint8_t pattern;

/* track buffers */
static uint8_t track_c1[TRACK_REC_SIZE];
static uint8_t track_c2[TRACK_REC_SIZE];
static uint8_t track_c3[TRACK_REC_SIZE];

/* track pointers */
static uint8_t *track1, *track2, *track3;

/* replayer state */
static struct replayer replayer_state;

/* PSG registers */
static struct ay_reg_map ay_regs;

/* data Tables */

uint8_t tone_table[12][8];
uint8_t vibrato_sine_table[32];
uint8_t vibrato_triangle_table[32];
uint8_t vibrato_pulse_table[32];
uint8_t volume_table[16*8];

/**
 * replay_route:
 *     output data to PSG
 */
void replay_route()
{
    psg_set_all(&ay_regs);
}

void replay_check_pattern_end()
{
    // TODO
}


void replay_decode_channel()
{
    // decode_note
    // decode_rest
    // decode_vol
    // decode_ins
    // decode_delay
    // decode_cmd
}

/**
 * replay_decode_data:
 *    process the pattern data
 */
void replay_decode_data()
{

}

void replay_play()
{
    if (replayer_state.mode != REPLAYER_PLAY)
        return;

    if(--replayer_state.speed_timer > 0) {
        replay_check_pattern_end();
    }

    // replayer_state.speed_subtimer = ;
    // replayer_state.speed_timer = ;

    replay_decode_data();

}

void replay_init()
{
    // get start speed

    // track pointers are set to the song_base + 3

    replayer_state.vibrato_table = vibrato_sine_table;
    //replayer_state.tone_table = tone_table;
    replayer_state.speed_timer = 1;

    replay_route();

    replayer_state.mode = REPLAYER_PLAY;
}


void music_isr(void)
{
    replay_route();
    replay_play();
}

void music_init(uint16_t* song)
{
    // need to place the player ISR in hook
    sys_proc_register(music_isr);

    replay_init();

    //replay_loadsong(song);

    pattern = 0;

    // depack note table

    // create note tables

    // create volume table

}

void music_mute(void)
{

}
