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
#include <stdarg.h>

#pragma CODE_PAGE 2

#define DEBUG
#ifdef DEBUG

/**
 * send a char to the debug port
 */
static void putchar(char c) __nonbanked
{
	c;

	__asm
	ld a,#0x63
	out (0x2e),a
	ld a, 4(ix)
	out (0x2f),a
	__endasm;
}

/*
 * Print an unsigned integer in base b.
 */
void printn(unsigned int n, char b) __nonbanked
{
	unsigned int a,r;

	if (a = n / b)
		printn(a, b);
	r = n % b;
	if (b > 10 && r > 8)
		putchar(r - 10 + 'A');
	else
		putchar(r + '0');

}

static void vprintk(char *fmt, va_list args) __nonbanked
{
	register char *s;
	register char c;
	unsigned int d;

 loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if (c == 'd' || c == 'l' || c == 'x') {
		d = va_arg(args, unsigned int);
		printn(d, c == 'x' ? 16 : 10);
	}
	if (c == 's') {
		s = va_arg(args, char *);
		while (c = *s++)
			putchar(c);
	}
	goto loop;
}

/**
 * log
 *   debug log trough printer port,
 *   useful when running in openmsx
 */
void log(int level, char *fmt, ...)
{
	va_list args;

	printn(sys_gettime_secs(), 10);
	putchar(':');
	printn(sys_gettime_msec(), 10);
	putchar(' ');
	if (level <= LOGLEVEL) {
		putchar('[');
		switch (level) {
			case LOG_ERROR:
				putchar('E');
				break;
			case LOG_DEBUG:
				putchar('D');
				break;
			case LOG_WARNING:
				putchar('W');
				break;
			case LOG_INFO:
				putchar('I');
				break;
			case LOG_VERBOSE:
				putchar('V');
				break;
			case LOG_ENTRY:
				putchar('>');
				break;
			case LOG_EXIT:
				putchar('<');
				break;
		}
		putchar (']');
		putchar (' ');
		va_start(args, fmt);
		vprintk(fmt, args);
		va_end(args);
		putchar ('\r');
	}
}
#endif
