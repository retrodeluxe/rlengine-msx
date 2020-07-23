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

#include "msx.h"
#include "sys.h"
#include "log.h"

#pragma CODE_PAGE 2

static unsigned int sys_secs;
static unsigned int sys_msec;
static unsigned int sys_ticks;
static uint8_t i;

struct sys_pr {
    uint8_t np;
    struct sys_proc  proc[MAX_PROCS];
} sys_procs;

void sys_reboot()
{
        __asm
        call 0x0000
        __endasm;
}

void sys_disable_kbd_click()
{
	__asm
	xor a
	ld (SYS_CLIKSW),a
	__endasm;
}

uint8_t sys_get_key(uint8_t line) __nonbanked
{
        line;

        __asm
        ld a,4(ix)
        call 0x0141
        ld h,#0x00
        ld l,a
        __endasm;
}

uint8_t sys_get_char(void)
{
        __asm
        call 0x009f
        ld h,#0x00
        ld l,a
        __endasm;
}

uint8_t sys_get_trigger(uint8_t port) __nonbanked
{
        port;

        __asm
        ld a,4(ix)
        call 0x00d8
        ld l,a
        __endasm;
}

uint8_t sys_get_stick(uint8_t port) __nonbanked
{
        port;

        __asm
        ld a,4(ix)
        call 0x00d5
        ld l,a
        __endasm;
}

void sys_memcpy(uint8_t *dst, uint8_t *src, uint16_t size) __nonbanked
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

// void sys_memset (void *dst, uint8_t c, uint16_t size)
// {
// 	dst;
// 	c;
// 	size;
//
// 	__asm
// 	ld l,4(ix)
//         ld h,5(ix)
//         ld e,l
//         ld d,h
// 	inc de
// 	ld a,6(ix)
// 	ld (hl),a
//         ld c,7(ix)
//         ld b,8(ix)
//         ldir
// 	__endasm;
// }


/**
 * sys_irq_register:
 *      register a function to be run from the interrupt handler
 *      any registered caller must run with _interrupts disabled_
 */
void sys_irq_register(void (*func))
{
	// should check if the function is already there maybe?
	sys_procs.proc[sys_procs.np++].func = func;
}

static void sys_remove_callback(uint8_t index) __nonbanked
{
	for (i = index;i < sys_procs.np - 1; i++) {
		sys_procs.proc[i].func = sys_procs.proc[i + 1].func;
	}
	sys_procs.np--;
}

/**
 * sys_irq_unregister
 */
void sys_irq_unregister(void (*func)) __nonbanked
{
	for (i = 0; i < sys_procs.np; i++) {
		if(sys_procs.proc[i].func == func)
			sys_remove_callback(i);
	}
	/* ignore if not found */
}


/**
 * sys_irq_handler:
 *      irq handler using
 */
static void sys_irq_handler(void) __nonbanked
{
    sys_ticks++;
    sys_msec = sys_msec + MSEC_PER_TICK;
    if (sys_msec > 1000) {
        sys_msec = 0;
        sys_secs++;
    }
    for (i=0; i < sys_procs.np; i++)
        (sys_procs.proc[i].func)();
}

/**
 * sys_irq_init
 *      register irq handler using BIOS hook
 */
void sys_irq_init()
{
    uint8_t lsb, msb;
    void (*handler)();
    uint8_t *hook =(uint8_t *) BIOS_INT_HOOK;

    sys_msec = 0;
    sys_secs = 0;
    sys_ticks = 0;
    sys_procs.np = 0;

    handler = sys_irq_handler;
    lsb=(uint8_t) handler & 255;
    msb=(uint8_t) (((int)handler >> 8) & 255);

    __asm di __endasm;
    *(hook)   = 0xc3; /* jp  */
    *(hook+1) = lsb;
    *(hook+2) = msb;
    *(hook+3) = 0xc9; /* ret */
    *(hook+4) = 0xc9; /* ret */
    __asm ei __endasm;
}


/**
 * sys_sleep_ms
 *   FIXME: currently can sleep a max of 1000ms
 */
void sys_sleep_ms(uint16_t msecs) __nonbanked
{
    uint16_t start_ms = sys_msec;
    while (sys_msec - start_ms < msecs) {
    };
}

void sys_sleep(uint16_t secs) __nonbanked
{
    uint16_t start_secs = sys_secs;
    while (sys_secs - start_secs < secs) {
    };
}

uint16_t sys_gettime_secs() __nonbanked
{
    return sys_secs;
}

uint16_t sys_gettime_msec() __nonbanked
{
    return sys_msec;
}

uint16_t sys_get_ticks() __nonbanked
{
    return sys_ticks;
}

/**
 *  LFSR RNG
 */
uint8_t rng_seed[8];

uint8_t sys_rand_init(uint8_t *seed) __nonbanked
{
	sys_memcpy(rng_seed, seed, 8);
}

uint8_t sys_rand() __nonbanked
{
	__asm
	ld hl,#_rng_seed+4
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld c,(hl)
	inc hl
	ld a,(hl)
	ld b,a
	rl e
	rl c
	rl e
	rl c
	rl e
	rl c
	ld h,a
	rl e
	rl c
	xor b
	rl e
	xor h
	xor c
	xor d
	ld hl,#_rng_seed+6
	ld de,#_rng_seed+7
	ld bc,#7
	lddr
	ld (de),a
	ld l,a
	__endasm;
}

bool sys_is60Hz() __nonbanked
{
	uint8_t *info =(uint8_t *) SYS_INFO1;
	return (*(info) & 128) == 0;
}
