/**
 *
 * Copyright (C) Retro DeLuxe 2017, All rights reserved.
 *
 */

#define DEBUG
#include "msx.h"
#include "sys.h"
#include "log.h"
#include "mem.h"


uint8_t *alloc1, *alloc2, *alloc3, *alloc4, *alloc5;

void main()
{

	log_e("init memory\n");

	mem_init();

	alloc1 = mem_alloc(1024);
	log_e("alloc 1 %x\n", alloc1);

	alloc2 = mem_alloc(1024);
	log_e("alloc 2 %x\n", alloc2);

	alloc3 = mem_alloc(1024);
	log_e("alloc 3 %x\n", alloc3);

	mem_free(alloc2);

	alloc2 = mem_alloc(512);
	log_e("alloc 2 %x\n", alloc2);

	alloc4 = mem_alloc(256);
	log_e("alloc 4 %x\n", alloc4);

	alloc5 = mem_calloc(10, 1024);
    log_e("alloc 5 %x\n", alloc5);

	alloc5 = mem_calloc(4, 1024);
    log_e("alloc 5 %x\n", alloc5);

	for(;;);
}

