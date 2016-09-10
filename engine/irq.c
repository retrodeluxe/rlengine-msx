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


/*
 * irq related functions
 *
 */
#define  TIMER_HOOK             0xFD9F
#define  TICKS_CYCLE            60              // assume 60Hz
#define  SECS_CYCLE             60
#define  MAX_TPROC              50

struct irq_timed_proc {
        byte                            tic_rate;
        byte                            sec_rate;
        void                            (*func)();
};

struct irq_timed_procs {
        struct irq_timed_proc        p[MAX_TPROC];
        byte                          n;
} timed_procs;

static byte sys_tics;
static byte sys_secs;
static byte i;
static byte first_irq;

// Note : we must have a mechanism to split the work to be done during
//        interrupt processes across different calls, if needed.
//        At least for map decompression.
static void irq_handler(void)
{
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

        /* - be careful of not enabling irq when calling from here.
         * - during the VBLANK period, you can only send around
         *   2500 bytes to VRAM if you exceed that amount, better wait
         *   until the next cycle or you will generate a CPU stall.
         */
        for (i=0;i<timed_procs.n;i++){
                if (sys_secs % timed_procs.p[i].sec_rate == 0)
                        if (sys_tics % timed_procs.p[i].tic_rate == 0)
                                (timed_procs.p[i].func)();
        }
        if (--sys_tics==0) {
                sys_tics=TICKS_CYCLE;
                if (++sys_secs==0)
                        sys_secs=SECS_CYCLE;
        }

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

void irq_init(void)
{
        timed_procs.n=0;
}

void irq_start(void)
{
        byte lsb, msb;
        void (*handler)();
        byte *hook = TIMER_HOOK;

        handler = irq_handler;
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

void irq_register(void (*func), byte tic_rate, byte sec_rate)
{
        timed_procs.p[timed_procs.n].func=func;
        timed_procs.p[timed_procs.n].tic_rate=tic_rate;
        timed_procs.p[timed_procs.n].sec_rate=sec_rate;
        timed_procs.n++;
}

/*
void irq_unregister(byte id)
{
}

void irq_schedule_work(void (*func), int delay_tics)
{
}
*/

