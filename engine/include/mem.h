/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2022 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#ifndef _MEM_H_
#define _MEM_H_

#include <stddef.h>
#include <stdbool.h>

/** Defines the bottom of the Stack */
#define HEAP_MAX 0xF000

typedef struct HeapBlock HeapBlock;

struct HeapBlock 
{
    size_t size;
    bool is_free;
    HeapBlock *next;
};

void mem_init();
void *mem_alloc(size_t size);
void *mem_calloc(size_t num, size_t nsize);
void mem_free(void *block);

#endif