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
#include "sys.h"
#include "log.h"

static unsigned int sys_secs;
static unsigned int sys_msec;
static byte i;

struct sys_pr {
    byte np;
    struct sys_proc  proc[MAX_PROCS];
} sys_procs;

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

/**
 * sys_proc_register:
 *      register a function to be run from the interrupt handler
 */
void sys_proc_register(void (*func))
{
    sys_procs.proc[sys_procs.np++].func = func;
}

/**
 * sys_irq_handler:
 *      irq handler using
 */
static void sys_irq_handler(void)
{
        // I think I should save more registers here...
        __asm
        di
        in a,(#0x99)
        push af
        push bc
        push de
        push hl
        push ix
        push iy
        __endasm;

        sys_msec = sys_msec + MSEC_PER_TICK;
        if (sys_msec > 1000) {
            sys_secs++;
        }

        for (i=0; i < sys_procs.np; i++)
            (sys_procs.proc[i].func)();

        __asm
        pop iy
        pop ix
        pop hl
        pop de
        pop bc
        pop af
        ei
        __endasm;
}

/**
 * sys_irq_init
 *      register irq handler using BIOS hook
 */
void sys_irq_init()
{
        byte lsb, msb;
        void (*handler)();
        byte *hook = BIOS_INT_HOOK;

        sys_msec = 0;
        sys_secs = 0;
        sys_procs.np = 0;

        handler = sys_irq_handler;
        lsb=(byte) handler & 255;
        msb=(byte) (((int)handler >> 8) & 255);

        asm__di;
        *(hook)   = 0xc3; /* jp  */
        *(hook+1) = lsb;
        *(hook+2) = msb;
        *(hook+3) = 0xc9; /* ret */
        *(hook+4) = 0xc9; /* ret */
        asm__ei;
}


/**
 * sys_sleep:
 *      just wait in the "main thread". During this time only interrupt driven execution
 *      will happen.
 */
void sys_sleep(unsigned int time_ms)
{
    unsigned int start_ms = sys_msec;
    while (sys_msec - start_ms < time_ms) {
    };
}
