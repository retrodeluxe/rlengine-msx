/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */


 // PICO-8 PSG music format

// Sound effects

// The sound effects section begins with the delimiter __sfx__.
//
// Sound data is stored in the .p8 file as 64 lines of 168 hexadecimal digits (84 bytes, most significant nybble first), one line per sound effect (0-63).
//
// The byte values (hex digit pairs, MSB) are as follows:
//
// byte 0: The editor mode: 0 for pitch mode, 1 for note entry mode.
// byte 1: The note duration, in multiples of 1/128 second.
// byte 2: Loop range start, as a note number (0-63).
// byte 3: Loop range end, as a note number (0-63).
// bytes 4-84: 32 notes
// Each note is represented by 20 bits = 5 nybbles = 5 hex digits. (Two notes use five bytes.) The nybbles are:
//
// nybble 0-1: pitch (0-63): c-0 to d#-5, chromatic scale
// nybble 2: waveform (0-F): 0 sine, 1 triangle, 2 sawtooth, 3 long square, 4 short square, 5 ringing, 6 noise, 7 ringing sine; 8-F are the custom waveforms corresponding to sound effects 0 through 7 (PICO-8 0.1.11 "version 11" and later)
// nybble 3: volume (0-7)
// nybble 4: effect (0-7): 0 none, 1 slide, 2 vibrato, 3 drop, 4 fade_in, 5 fade_out, 6 arp fast, 7 arp slow; arpeggio commands loop over groups of four notes at speed 2 (fast) and 4 (slow)

// Music patterns

// The sound effects section begins with the delimiter __music__.
//
// Music patterns are represented in the .p8 file as 64 lines, one for each pattern. Each line consists of a hex-encoded flags byte (two digits), a space, and four hex-encoded one-byte (MSB-first) sound effect IDs.
//
// The flags byte has three flags on the lower bits (the higher five bits are unused):
//
// 0: begin pattern loop
// 1: end pattern loop
// 2: stop at end of pattern
// The sound effect ID is in the range 0-63. If a channel is silent for a song pattern, its number is 64 + the channel number (0x41, 0x42, 0x43, or 0x44).

#include "msx.h"
#include "log.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"
#include "blit.h"
#include "phys.h"
#include "ascii8.h"

#include "pico8sfx.h"

#pragma CODE_PAGE 4

MusicPattern *music; //-- 8Kb this alone 8448
SoundEffect *sound_effect; //-- 320

void unpack_sfx(uint8_t *sfx)
{
  int i,j;
  // try to read some...
  sound_effect = (SoundEffect *)sfx;

  // just need to keep a pointer and read
  // sequentially at the specified tempo

  // in msx each step is either 20ms or 16ms

  // in pico8 minimum step is 7.8ms
  //
  // we cannot sypport the fastest tempo
  // unless we play two notes in 1 interrupt
  // this maybe interesting for sound effects but
  // not in general.

  // other limitations is that we do not support
  // custom waveforms, and the effects need to be
  // implemented separately...

  for (i = 0; i < 64; i++) {
    log_e("sfx %d\n", i);
    log_e("sfx mode: %x\n", sound_effect->mode);
    log_e("sfx len: %x\n", sound_effect->length);
    log_e("sfx start: %x\n", sound_effect->loop_start);
    log_e("sfx end: %x\n", sound_effect->loop_end);

    for (j=0; j<16;j++) {
      log_e("sfx note %d\n", j);
      log_e("sfx pitch_a: %x\n", sound_effect->notes[j].pitch_a);
      log_e("sfx wave_a: %x\n", sound_effect->notes[j].wave_a);
      log_e("sfx vol_a: %x\n", sound_effect->notes[j].vol_a);
      log_e("sfx effect_a: %x\n", sound_effect->notes[j].effect_a);
      log_e("sfx pitch_b: %x\n", sound_effect->notes[j].pitch_b);
    }

    log_e("-----\n");
    sound_effect++;
  }
  //log_e("sfx mode: %d", sfx->node);
}

void unpack_music(uint8_t *music)
{

}

void play_music() __nonbanked
{
}
