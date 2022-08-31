/**
 *
 * Copyright (C) Retro DeLuxe 2022, All rights reserved.
 *
 */
#include "msx.h"
#include "sys.h"
#include "timer.h"
#include "log.h"
#include <stdlib.h>

Timer timer_one, timer_two;
uint16_t delay1, delay2;

void timer_cb1(uint8_t data);
void timer_cb2(uint8_t data);

void main()
{
	sys_irq_init();
	timer_init();

	delay1 = timer_msecs_to_tics(5000);
	delay2 = timer_msecs_to_tics(10000);

	timer_define(&timer_one, timer_cb1, delay1, true);
	timer_define(&timer_two, timer_cb2, delay2, false);

	timer_add(&timer_one, 66);
	timer_add(&timer_two, 77);

	for(;;);
}

void timer_cb1(uint8_t data)
{
	log_i("timer 1 expired %d\n", data);
}

void timer_cb2(uint8_t data)
{
	log_i("timer 2 expired %d\n", data);
}
