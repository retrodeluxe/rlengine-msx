/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */
#include "msx.h"

typedef struct NotePair NotePair;

struct NotePair {
  uint8_t pitch_a;
  uint32_t wave_a :4;
  uint32_t vol_a :4;
  uint32_t effect_a :4;
  uint32_t pitch_b :8;
  uint32_t wave_b :4;
  uint32_t vol_b :4;
  uint32_t effect_b :4;
};

typedef struct SoundEffect SoundEffect;

struct SoundEffect {
  uint8_t mode;

  /** note duration in 1/128 sec steps (7ms) */
  uint8_t length;

  /** loop range start note (0-63) */
  uint8_t loop_start;

  /** loop range end note (0-63) */
  uint8_t loop_end;

  /** Note array */
  NotePair notes[16];
};

typedef struct MusicPattern MusicPattern;

struct MusicPattern {

  /** pattern flags */
  uint8_t flags;

  /** sound effect id's for every channel */
  uint8_t sound[4];
};

extern void unpack_sfx(uint8_t *sfx);
extern void unpack_music(uint8_t *music);
extern void play_music();
