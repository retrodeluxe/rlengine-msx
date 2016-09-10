/*
 * RetroDeLuxe Engine MSX1
 *
 * Copyright (C) 2013 Enric Geijo
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

void sys_reboot()
{
        __asm
        call 0x0000
        __endasm;
}

byte sys_get_key(byte line)
{
        line;

        __asm
        ld a,4(ix)
        call 0x0141
        ld h,#0x00
        ld l,a
        __endasm;
}

byte sys_get_stick(byte port)
{
        port;

        __asm
        ld a,4(ix)
        call 0x00d5
        ld l,a
        __endasm;
}

void sys_memcpy(byte *dst, byte *src, uint size)
{
        src;
        dst;
        size;

        __asm
        ld l,6(ix)
        ld h,7(ix)
        ld e,4(ix)
        ld d,5(ix)
        ld c,8(ix)
        ld b,9(ix) ;#0x00
        ldir
        __endasm;
}

