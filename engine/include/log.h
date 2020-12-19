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

#ifndef _MSX_H_LOG
#define _MSX_H_LOG

#define LOG_ERROR 0
#define LOG_DEBUG 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_VERBOSE 4
#define LOG_ENTRY 5
#define LOG_EXIT 6

#define LOGLEVEL 7

#ifdef NDEBUG
#define log_d(_fmt, ...)
#define log_w(_fmt, ...)
#define log_i(_fmt, ...)
#define log_v(_fmt, ...)
#define log_e(_fmt, ...)
#define log_entry(_fmt, ...)
#define log_exit(_fmt, ...)

#else
extern void log(int level, char *fmt, ...) __nonbanked;
extern void dump_vram(int start_addr, int end_addr);

#define log_d(_fmt, ...) log(LOG_DEBUG, _fmt, ##__VA_ARGS__)
#define log_w(_fmt, ...) log(LOG_WARNING, _fmt, ##__VA_ARGS__)
#define log_i(_fmt, ...) log(LOG_INFO, _fmt, ##__VA_ARGS__)
#define log_v(_fmt, ...) log(LOG_VERBOSE, _fmt, ##__VA_ARGS__)
#define log_e(_fmt, ...) log(LOG_ERROR, _fmt, ##__VA_ARGS__)
#define log_entry(_fmt, ...) log(LOG_ENTRY, _fmt, ##__VA_ARGS__)
#define log_exit(_fmt, ...) log(LOG_EXIT, _fmt, ##__VA_ARGS__)

#define assert(x) ((x) ? (void)0 : __assert(#x, __func__, __FILE__, __LINE__))

#endif /* NDEBUG */

#endif
