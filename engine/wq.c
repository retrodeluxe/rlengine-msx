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
        //log_d("run head=%d\n",wq.head);
        (wq.cq[wq.head].func)();
        if (++wq.head > WQ_BUF_SIZE - 1) {
            wq.head = 0;
        }
    }
}

static void wq_delayed_run()
{
    uint secs = sys_gettime_secs();
    uint msec = sys_gettime_msec();
    signed int delta_secs, delta_msec;
    if (delayed_wq.head != delayed_wq.tail) {
        delta_secs = delayed_wq.cq[delayed_wq.head].alarm_secs - secs;
        delta_msec = delayed_wq.cq[delayed_wq.head].alarm_msec - msec;
        if (delta_secs < 0 || (delta_secs <= 0 && delta_msec < 50)) {
            //log_d("run secs=%d %d\n",delta_secs, delta_msec);
            (delayed_wq.cq[delayed_wq.head].func)();
        } else {
            //log_d("requeue\n");
            queue_delayed_work(&delayed_wq.cq[delayed_wq.head], delta_secs, delta_msec, delayed_wq.cq[delayed_wq.head].data);
        }
        if (++delayed_wq.head > WQ_BUF_SIZE - 1) {
            delayed_wq.head = 0;
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

int queue_work(struct work_struct *work, char data)
{
    if (wq.tail < (wq.head - 1) || wq.tail >= wq.head) {
        wq.cq[wq.tail].func = work->func;
        wq.cq[wq.tail].data = data;
        if (++wq.tail > WQ_BUF_SIZE - 1) {
            wq.tail = 0;
        }
        return 0;
    }
    return 1;
}


int queue_delayed_work(struct work_struct *work, uint delay_secs, uint delay_msec, char data)
{
    uint secs = sys_gettime_secs();
    uint msec = sys_gettime_msec();
    if (delayed_wq.tail < (delayed_wq.head - 1) ||
            delayed_wq.tail >= delayed_wq.head) {
        delayed_wq.cq[delayed_wq.tail].func = work->func;
        delayed_wq.cq[delayed_wq.tail].data = data;
        if (delay_secs != 0 || delay_msec != 0) {
            if (msec + delay_msec > 1000) {
                 delayed_wq.cq[delayed_wq.tail].alarm_secs = secs + delay_secs + 1;
                delayed_wq.cq[delayed_wq.tail].alarm_msec = msec + delay_msec - 1000;
            } else {
                delayed_wq.cq[delayed_wq.tail].alarm_secs = secs + delay_secs;
                delayed_wq.cq[delayed_wq.tail].alarm_msec = msec + delay_msec;
            }
        }
        //log_d("queue_delayed [%d, %d] (%d, %d)\n",  delayed_wq.tail, delayed_wq.head, delayed_wq.cq[delayed_wq.tail].alarm_secs,delayed_wq.cq[delayed_wq.tail].alarm_msec);
        if (++delayed_wq.tail > WQ_BUF_SIZE - 1) {
            delayed_wq.tail = 0;
        }
        return 0;
    }
    return 1;
}