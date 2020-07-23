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

#ifndef _MSX_H_
#define _MSX_H_

#define DEBUG

#include <stdint.h>
#include <stdbool.h>

#define VDP_DR		0x0006
#define VDP_DW		0x0007
#define SYS_INFO1	0x002B
#define SYS_INFO2	0x002C
#define SYS_INFO3	0x002D
#define SYS_CLIKSW	0xF3DB

#define BIOS_DISSCR 	0x0041
#define BIOS_ENASCR 	0x0044
#define BIOS_CHGMOD 	0x005F
#define BIOS_WRTPSG	0x0093
#define BIOS_RDPSG	0x0096


#define sys_irq_disable()                 __asm di __endasm
#define sys_irq_enable()                 __asm ei __endasm
#define sys_wait_vsync()		__asm halt __endasm
#endif				/* _MSX_H_ */
