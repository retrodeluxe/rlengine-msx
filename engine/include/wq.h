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
 
#ifndef _WQ_H_
#define _WQ_H_


#define INIT_WORK(WORK, FUNC)     (WORK).func = (FUNC);\
                                  (WORK).pending = 0;

#define WQ_BUF_SIZE 20

struct work_struct {
    void (*func)();
    unsigned char data;
    unsigned char pending;
    uint16_t alarm_secs;
    uint16_t alarm_msec;
};

struct work_queue {
    uint8_t head;
    uint8_t tail;
    struct work_struct *cq[WQ_BUF_SIZE];
};

extern void wq_start();
extern int queue_work(struct work_struct *work);
extern int queue_delayed_work(struct work_struct *work, uint16_t delay_secs, uint16_t delay_msec);
#endif
