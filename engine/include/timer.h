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
#include <stdint.h>
#include <stdbool.h>
#include "list.h"

#ifndef _TIMER_H_
#define _TIMER_H_

typedef struct Timer Timer;
struct Timer {
  List list;

  /* callback and data */
  void (*func)(uint8_t data);
  uint8_t data;

  /* expiration time in sys_ticks (1/50 or 1/60 secs) */
  uint16_t expires;
};

extern void timer_init();
extern void timer_define(Timer *timer, void (*func)(), uint16_t expires);
extern void timer_add(Timer *timer, uint8_t data);
extern void timer_del(Timer *timer);
#endif
