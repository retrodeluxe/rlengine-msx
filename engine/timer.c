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
#include "log.h"

List timer_list;
List *item;
Timer *timer;

static void timer_thread() __nonbanked
{
  list_for_each(item, &timer_list) {
    timer = list_entry(item, Timer, list);
    timer->count--;
    if (0 == timer->count) {
      (timer->func)(timer->data);
      if (timer->repeat)
        timer->count = timer->expires;
      else
        list_del(&timer->list);
    }
  }
}

/**
 * Initialize the timer module.
 *
 */
void timer_init()
{
  INIT_LIST_HEAD(&timer_list);
  sys_irq_register(timer_thread);
}

/**
 * Define a Timer
 *
 * :param timer: Timer object
 * :param func: Callback function to be called on expiration
 * :param expires: Time to expiration in tics (50/60Hz)
 * :param repeat: True to repeat after expiration
 */
void timer_define(Timer *timer, void (*func)(uint8_t data),
                  uint16_t expires, bool repeat)
{
  timer->func = func;
  timer->count = timer->expires = expires;
  timer->repeat = repeat;
  INIT_LIST_HEAD(&timer->list);
}

/**
 * Activate a Timer
 *
 * :param timer: reference to the Timer to activate
 * :param data: byte to be passed on to the timer callback
 */
void timer_add(Timer *timer, uint8_t data)
{
  timer->data = data;
  list_add(&timer->list, &timer_list);
}

/**
 * Deactivate a Timer 
 *
 * :param timer: reference to the timer to deactivate
 */
void timer_del(Timer *timer) {
  list_del(&timer->list);
}

/**
 * Convert time to tics taking into account interrupt rate
 * on the machine running the Timer.
 *
 * :param msecs: miliseconds
 * :returns uint16_t: tics
 */
uint16_t timer_msecs_to_tics(uint16_t msecs)
{
  if (sys_is60Hz()) {
    return msecs / 16;
  } else {
    return msecs / 20;
  }
}