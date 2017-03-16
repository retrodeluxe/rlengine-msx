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
 *  Driver for the General Instruments AY-3-891x Programmable Sound Generator
 *
 *  (ASCII Art from MSX Technical Handbook typed by Konami Man)

 -----------------------------------------------------------------------------
 |                       Bit |     |     |     |     |     |     |     |     |
 |                           | B7  | B6  | B5  | B4  | B3  | B2  | B1  | B0  |
 | Register                  |     |     |     |     |     |     |     |     |
 |---------------------------+-----------------------------------------------|
 |    R0    | Channel A note |            8 low order bits                   |
 |----------|                |-----------------------------------------------|
 |    R1    | Dividing rate  |  x     x     x     x  |   4 high order bits   |
 |----------+----------------+-----------------------------------------------|
 |    R2    | Channel B note |           8 low order bits                    |
 |----------|                |-----------------------------------------------|
 |    R3    | Dividing rate  |  x     x     x     x  |   4 high order bits   |
 |----------+----------------+-----------------------------------------------|
 |    R4    | Channel C note |           8 low order bits                    |
 |----------|                |-----------------------------------------------|
 |    R5    | Dividing rate  |  x     x     x     x  |   4 high order bits   |
 |----------+----------------+-----------------------------------------------|
 |    R6    | Noise div. rate|  x     x     x  |                             |
 |----------+----------------+-----------------------------------------------|
 |          |                |  IN/OUT   |      NOISE*     |  TONE*          |
 |    R7    | Enable*        |-----------+-----------------+-----------------|
 |          |                | IOB | IOA |  C  |  B  |  A  |  C  |  B  |  A  |
 |----------+----------------+-----------------+-----+-----------------------|
 |    R8    | Chan. A volume |  x     x     x  |  M  |                       |
 |----------+----------------+-----------------+-----+-----------------------|
 |    R9    | Chan. B volume |  x     x     x  |  M  |                       |
 |----------+----------------+-----------------+-----+-----------------------|
 |    R10   | Chan. C volume |  x     x     x  |  M  |                       |
 |----------+----------------+-----------------------------------------------|
 |    R11   |                |            8 low order bits                   |
 |----------| Envelope Cycle |-----------------------------------------------|
 |    R12   |                |           8 high order bits                   |
 |----------+----------------+-----------------------------------------------|
 |    R13   | Env. wave shape|  x     x     x     x  |                       |
 |----------+----------------+-----------------------------------------------|
 |    R14   | I/O port A     |                                               |
 |----------+----------------+-----------------------------------------------|
 |    R15   | I/O port B     |                                               |
 -----------------------------------------------------------------------------


  Table 5.1  Setting the tone frequency (scale data)

 ----------------------------------------------------------------
 |    Octave    |     |     |     |     |     |     |     |     |
 |              |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
 | Note         |     |     |     |     |     |     |     |     |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      C       | D5D | 6AF | 357 | 1AC |  D6 |  6B |  35 |  1B |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      C#      | C9C | 64E | 327 | 194 |  CA |  65 |  32 |  19 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      D       | BE7 | 5F4 | 2FA | 17D |  BE |  5F |  30 |  18 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      D#      | B3C | 59E | 2CF | 168 |  84 |  5A |  2D |  16 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      E       | A9B | 54E | 2A7 | 153 |  AA |  55 |  2A |  15 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      F       | A02 | 501 | 281 | 140 |  A0 |  50 |  28 |  14 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      F#      | 973 | 4BA | 25D | 12E |  97 |  4C |  26 |  13 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      G       | 8EB | 476 | 23B | 11D |  8F |  47 |  24 |  12 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      G#      | 88B | 436 | 21B | 10D |  87 |  43 |  22 |  11 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      A       | 7F2 | 3F9 | 1FD |  FE |  7F |  40 |  20 |  10 |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      A#      | 780 | 3C0 | 1E0 |  F0 |  78 |  3C |  1E |   F |
 |--------------+-----+-----+-----+-----+-----+-----+-----+-----|
 |      B       | 714 | 38A | 1C5 |  E3 |  71 |  39 |  1C |   E |
 ----------------------------------------------------------------

*/

#include "msx.h"


#define psg_set   #0xa0
#define PSG_AMP_VOL_MASK    0x0F
#define PSG_AMP_USEENVELOPE 0x10
#define PSG_MIX_NOISE_A     0x20
#define PSG_MIX_NOISE_B     0x10
#define PSG_MIX_NOISE_C     0x08
#define PSG_MIX_TONE_A      0x04
#define PSG_MIX_TONE_B      0x02
#define PSG_MIX_TONE_C      0x01


void psg_write(uint8_t reg, uint8_t val)
{
        reg;
        val;

        __asm
        ld a,4(ix)
        ld e,5(ix)
        call 0x0093     ; WRTPSG
        __endasm;
}

void psg_set_all(struct ay_reg_map *regs)
{
    regs;

    __asm
    ld l,4(ix)
    ld h,5(ix)
    ld c,psg_set
    xor a
psg_loop:
    out (c),a
    inc c
    outi
    dec c
    inc a
    cp #13
    jr nz, psg_loop
    __endasm;
}

void psg_set_tone(unsigned int period, uint8_t chan)
{
    period;
    chan;

    __asm
    ld l,4(ix)
    ld h,5(ix)
    ld c,psg_set
    ld a,6(ix)
    sla a
    ld b,a
    out (c),a
    inc c
    out (c),l
    dec c
    ld a,b
    inc a
    out (c),a
    inc c
    out (c),h
    __endasm;

}

void psg_set_noise(uint8_t period)
{
    period;

    __asm
    ld b,4(ix)
    ld c,psg_set
    ld a,#6
    out (c),a
    inc c
    out (c),b
    __endasm;
}

/* bits 6,7 must be left unchanged
 *  as wrong values might break some
 *  hardware
 */
void psg_set_mixer(uint8_t mixval)
{
    mixval;

    __asm
    ld h,4(ix)
    ld a,h
    and #0x3F
    ld h,a
    ld c,psg_set
    ld a,#7
    out (c),a
    inc c
    inc c
    in a,(c)
    or h
    dec c
    out (c),a
    __endasm;

}

void psg_set_vol(uint8_t chan, uint8_t vol)
{
    chan;
    vol;

    __asm
    ld a,4(ix)
    ld b,5(ix)
    ld c,psg_set
    add a,#8
    out (c),a
    inc c
    out (c),b
    __endasm;
}


/*
 * set envelope period and shape,
 *
 * EP = R12 * 256 + R11
 *
 * T = (256 * EP) / fc
 *	 = (256 * EP) / 1.787725 [MHz]
 *	 = 143.03493  * EP [micro second]
 *
 * shape is 4 bit and corresponds to:
 *
 *         -------------------------------------------------
 * R13    |  x     x       x    x  | B3  | B2  | B1  | B0  |
 *         -------------------------------------------------
 *                                           |
 *       ------------------------------------+
 *       |
 *       V
 * ---------------------------------------------------------
 * |                     |    :\                           |
 * |   0   0    x    x   |  __:  \______________________   |
 * |                     |                                 |
 * |                     |      /:                         |
 * |   0   1    x    x   |  __/  :______________________   |
 * |                     |                                 |
 * |                     |    :\  :\  :\  :\  :\  :\  :\   |
 * |   1   0    0    0   |  __:  \:  \:  \:  \:  \:  \:_   |
 * |                     |                                 |
 * |                     |    :\                           |
 * |   1   0    0    1   |  __:  \______________________   |
 * |                     |                                 |
 * |                     |    :\     / \     / \     / \   |
 * |   1   0    1    0   |  __:  \ /     \ /     \ /       |
 * |                     |        _____________________    |
 * |                     |    :\  :                        |
 * |   1   0    1    1   |  __:  \:                        |
 * |                     |                                 |
 * |                     |      /:  /:  /:  /:  /:  /:     |
 * |   1   1    0    0   |  __/  :/  :/  :/  :/  :/  :/    |
 * |                     |        ______________________   |
 * |                     |      /                          |
 * |   1   1    0    1   |  __/                            |
 * |                     |                                 |
 * |                     |      / \     / \     / \        |
 * |   1   1    1    0   |  __/     \ /     \ /     \ /    |
 * |                     |                                 |
 * |                     |      /:                         |
 * |   1   1    1    1   |  __/  :______________________   |
 * |                     |                                 |
 * ---------------------------------------------------------
 *                           |   |
 *                           +---+
 *                             T
 *
 */
void psg_set_envelope(unsigned int period, uint8_t shape)
{
    period;
    shape;

    __asm
    ld l,4(ix)
    ld h,5(ix)
    ld b,6(ix)
    ld c,psg_set
    ld a,#11
    out (c),a
    inc c
    out (c),l
    dec c
    inc a
    out (c),a
    inc c
    out (c),h
    dec c
    inc a
    out (c),a
    inc c
    out (c),b
    __endasm;
}
