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

#include "timer.h"
#include "msx.h"
#include "sys.h"

List timer_list;

static void timer_thread() {
  List *item;
  Timer *timer;
  list_for_each(item, &timer_list) {
    timer = list_entry(item, Timer, list);
    timer->expires--;
    if (0 == timer->expires) {
      (timer->func)(timer->data);
      list_del(&timer->list);
    }
  }
}

void timer_init() {
  INIT_LIST_HEAD(&timer_list);
  sys_irq_register(timer_thread);
}

void timer_define(Timer *timer, void (*func)(), uint16_t expires) {
  timer->func = func;
  timer->expires = expires;
  INIT_LIST_HEAD(&timer->list);
}

void timer_add(Timer *timer, uint8_t data) {
  timer->data = data;
  list_add(&timer->list, &timer_list);
}

void timer_del(Timer *timer) {
  list_del(&timer->list);
}
