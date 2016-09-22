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
#include "log.h"
#include <stdarg.h>

#define DEBUG
#ifdef DEBUG

/**
 * send a char to the debug port
 */
static void putchar(char c)
{
	__asm
	di
	ld a,#0x63
	out (0x2e),a
	ld a, 4(ix)
	out (0x2f),a
	ei
	__endasm;
}

/*
 * Print an unsigned integer in base b.
 */
void printn(unsigned int n, char b)
{
	register char a, r;

	if (a = n / b)
		printn(a, b);
	r = n % b;
	if (b > 10 && r > 8)
		putchar(r - 10 + 'A');
	else
		putchar(r + '0');

}

static void vprintk(char *fmt, va_list args)
{
	register char *s;
	register char c, d;

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

/**
 * rle inflate
 *  compression ratio is negligible unless you have many long
 *  bursts of repeated bytes in the source data, and requires
 *  doubling the buffer for decompression when used in combination
 *  with other methods.
 *
 *  -> out buffer size depends on compr. ratio
 */
void rle_inflate(byte * in, byte * out, uint len)
{
	char c;

	while (len > 0) {
		c = (char)*in++;

		if (c > 0) {
			sys_memset(out, *in++, c);

		} else if (c < 0) {
			c = (char)-c;
			sys_memcpy(out, in, c);
			in += c;
		}
		out += c;
		len -= c;
	}
}

/**
 * read a buffer an expand it using a 4x4 tile dictionary
 *  this provides a fixed ratio of 1/4 and decompression buffer
 *  size is fixed.
 *
 *  -> width of the map is needed
 *  -> out buffer needs to be 4 times data_size
 */

void blk_inflate(const byte * dict, const byte * in, byte * out, uint data_size,
		 byte w)
{
	byte col = 0;
	byte idx;
	byte *src;
	byte *dst = out;
	/* FIXME: optimize this */
	for (src = in - 4; src < in + data_size; src++) {
		idx = (*src) * 4;
		*(dst) = dict[idx];
		*(dst + 1) = dict[idx + 1];
		*(dst + w) = dict[idx + 2];
		*(dst + w + 1) = dict[idx + 3];
		col += 2;
		dst += 2;
		if (col >= w) {
			col = 0;
			dst += w;
		}
	}
}

/*
 * LZFX Adapted for MSX (z80) computers
 *
 * Copyright (c) 2013 Enric Geijo <enric.geijo at gmail.com>
 *
 * based on the original lzfx:
 *
 * Copyright (c) 2009 Andrew Collette <andrew.collette at gmail.com>
 * http://lzfx.googlecode.com
 *
 * Implements an LZF-compatible compressor/decompressor based on the liblzf
 * codebase written by Marc Lehmann.  This code is released under the BSD
 * license.  License and original copyright statement follow.
 *
 * Copyright (c) 2000-2008 Marc Alexander Lehmann <schmorp@schmorp.de>
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
*/

typedef unsigned char u8;

/* Predefined errors. */
#define LZFX_ESIZE      -1	/* Output buffer too small */
#define LZFX_ECORRUPT   -2	/* Invalid data for decompression */
#define LZFX_EARGS      -3	/* Arguments invalid (NULL) */

#define NULL 0

int lzfx_decompress(const void *ibuf, unsigned int ilen,
		    void *obuf, unsigned int *olen)
{

	u8 const *ip = (const u8 *)ibuf;
	u8 const *const in_end = ip + ilen;
	u8 *op = (u8 *) obuf;
	u8 const *const out_end = (olen == NULL ? NULL : op + *olen);

	unsigned int remain_len = 0;
	int rc;

	// if(olen == NULL) return LZFX_EARGS;
	// if(ibuf == NULL){
	//     if(ilen != 0) return LZFX_EARGS;
	//     *olen = 0;
	//     return 0;
	// }
	// if(obuf == NULL){
	//     if(olen != 0) return LZFX_EARGS;
	//     return lzfx_getsize(ibuf, ilen, olen);
	// }

	do {
		unsigned int ctrl = *ip++;

		/* Format 000LLLLL: a literal byte string follows, of length L+1 */
		if (ctrl < (1 << 5)) {
			ctrl++;

			if (op + ctrl > out_end) {
				--ip;	/* Rewind to control byte */
				goto guess;
			}
			if (ip + ctrl > in_end)
				return LZFX_ECORRUPT;

			do
				*op++ = *ip++;
			while (--ctrl);

			/*  Format #1 [LLLooooo oooooooo]: backref of length L+1+2
			   ^^^^^ ^^^^^^^^
			   A      B
			   #2 [111ooooo LLLLLLLL oooooooo] backref of length L+7+2
			   ^^^^^          ^^^^^^^^
			   A               B
			   In both cases the location of the backref is computed from the
			   remaining part of the data as follows:

			   location = op - A*256 - B - 1
			 */
		} else {

			unsigned int len = (ctrl >> 5);
			u8 *ref = op - ((ctrl & 0x1f) << 8) - 1;

			if (len == 7)
				len += *ip++;	/* i.e. format #2 */

			len += 2;	/* len is now #octets */

			if (op + len > out_end) {
				ip -= (len >= 9) ? 2 : 1;	/* Rewind to control byte */
				goto guess;
			}
			if (ip >= in_end)
				return LZFX_ECORRUPT;

			ref -= *ip++;

			if (ref < (u8 *) obuf)
				return LZFX_ECORRUPT;

			do
				*op++ = *ref++;
			while (--len);
		}

	} while (ip < in_end);

	*olen = op - (u8 *) obuf;

	return 0;

 guess:
	//rc = lzfx_getsize(ip, ilen - (ip-(u8*)ibuf), &remain_len);
	//if(rc>=0) *olen = remain_len + (op - (u8*)obuf);
	return 0;
}
