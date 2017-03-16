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
#include "wq.h"
#include "log.h"


/**
 * Simple work queue to schedule delayed execution on interrupt context.
 */

struct work_queue wq, delayed_wq;

/**
 * queue_run_next
 *      runs next work by delay
 */

static void wq_run()
{
    if (wq.head != wq.tail) {
        (wq.cq[wq.head]->func)();
        wq.cq[wq.head]->pending = 0;
        if (++wq.head > WQ_BUF_SIZE - 1) {
            wq.head = 0;
        }
    }
}

static void wq_delayed_run()
{
    uint16_t secs = sys_gettime_secs();
    uint16_t msec = sys_gettime_msec();
    signed int delta_secs, delta_msec;
    if (delayed_wq.head != delayed_wq.tail) {
        delta_secs = delayed_wq.cq[delayed_wq.head]->alarm_secs - secs;
        delta_msec = delayed_wq.cq[delayed_wq.head]->alarm_msec - msec;
        if (delta_secs < 0 || (delta_secs <= 0 && delta_msec < 50)) {
            (delayed_wq.cq[delayed_wq.head]->func)();
            delayed_wq.cq[delayed_wq.head]->pending = 0;
            if (++delayed_wq.head > WQ_BUF_SIZE - 1) {
                delayed_wq.head = 0;
            }
        } else {
            delayed_wq.cq[delayed_wq.head]->alarm_secs = delta_secs;
            delayed_wq.cq[delayed_wq.head]->alarm_msec = delta_msec;
        }
    }
}

void wq_start()
{
    wq.head = 0;
    wq.tail = 0;
    delayed_wq.head = 0;
    delayed_wq.tail = 0;
    sys_proc_register(wq_run);
    sys_proc_register(wq_delayed_run);
}

int queue_work(struct work_struct *work)
{
    if (wq.tail < (wq.head - 1) || wq.tail >= wq.head) {
        work->pending = 1;
        wq.cq[wq.tail] = work;
        if (++wq.tail > WQ_BUF_SIZE - 1) {
            wq.tail = 0;
        }
        return 0;
    }
    return 1;
}

int queue_delayed_work(struct work_struct *work, uint16_t delay_secs, uint16_t delay_msec)
{
    uint16_t secs = sys_gettime_secs();
    uint16_t msec = sys_gettime_msec();
    if (delayed_wq.tail < (delayed_wq.head - 1) ||
            delayed_wq.tail >= delayed_wq.head) {
        if (delay_secs != 0 || delay_msec != 0) {
            if (msec + delay_msec > 1000) {
                work->alarm_secs = secs + delay_secs + 1;
                work->alarm_msec = msec + delay_msec - 1000;
            } else {
                work->alarm_secs = secs + delay_secs;
                work->alarm_msec = msec + delay_msec;
            }
        }
        work->pending = 1;
        delayed_wq.cq[delayed_wq.tail] = work;
        if (++delayed_wq.tail > WQ_BUF_SIZE - 1) {
            delayed_wq.tail = 0;
        }
        return 0;
    }
    return 1;
}
