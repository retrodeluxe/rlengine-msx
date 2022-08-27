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

#include "mem.h"
#include "sys.h"
#include "log.h"

#pragma CODE_PAGE 2

static uint16_t heap_start, heap_end;

static HeapBlock *head,*tail; 

/*
 * Finds an existing free block of appriate size or returns NULL.
 * 
 * Beware that first available big-enough block is returned. If the block is
 * bigger than the requested size, the excess memory remains unused.
 * 
 * :param size: Size of the block 
 * :returns: HeapBlock* 
 */
static HeapBlock *find_free_block(size_t size)
{
	HeapBlock *curr = head;

	while(curr) {
		if (curr->is_free && curr->size >= size)
			return curr;
		curr = curr->next;
	}

	return NULL;
}

/*
 * Creates a new block and extends the end of the heap accordingly.
 * 
 * If the end of the heap exceeds MAX_HEAP returns, NULL. 
 * 
 * :param size: Size of the block 
 * :returns: HeapBlock* 
 */

static HeapBlock *new_block(size_t size)
{
    HeapBlock *new;

    new = (void *)heap_end;
    
    heap_end += sizeof(HeapBlock) + size;
    if (heap_end > HEAP_MAX) 
        return NULL;

	new->size = size;
	new->is_free = false;
	new->next = NULL;
    return new;
}

/**
 * Initializes the memory allocator by finding the amount of RAM
 * available between the end of the DATA segment and the bottom of the Stack. 
 * 
 */
void mem_init()
{
    /* heap starts af the end of the DATA segment */
    __asm
    ld hl,#s__DATA
    ld bc,#l__DATA
    adc hl, bc
    inc hl
    ld (#_heap_start),hl
    __endasm;

    if (HEAP_MAX - heap_start < 1024) {
        log_w("small heap : less then 1024 free\n");  
    }

    heap_end = heap_start;

    head = NULL;
    tail = NULL;
}

/**
 * Attemps to allocate a block of the requested size. 
 * 
 * :param size: Size of the block to be allocated 
 * :returns: void* pointer to the allocated block or NULL 
 */
void *mem_alloc(size_t size)
{
	HeapBlock *block;

	if (!size)
		return NULL;
	
	block = find_free_block(size);
	if (block) {
		block->is_free = false;
		return (void*)(block + 1);
	}
	
    block = new_block(size);
    if (!block) {
        log_e("Out of memory\n");
        return NULL;        
    }
	
    if (!head)
		head = block;
	
    if (tail)
		tail->next = block;
	
    tail = block;
	return (void*)(block + 1);
}


/**
 * Free a previously allocated block. 
 * 
 * :param block: Memory block to be freed.
 */
void mem_free(void *block)
{
	HeapBlock *header, *tmp;

	if (!block)
		return;

	header = (HeapBlock*)block - 1;

	if ((uint16_t)block + header->size == heap_end) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->next == tail) {
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
        heap_end -= sizeof(HeapBlock) - header->size;
		return;
	}
	header->is_free = true;
}

/**
 * Allocates an array of elements and sets to zero. 
 * 
 * :param num: Number of elements
 * :param nsize: Size of each element
 * :returns: void* pointer to the allocated block or NULL 
 */

void *mem_calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;

	if (!num || !nsize)
		return NULL;

	size = num * nsize;

	/* check mul overflow */
	if (nsize != size / num)
		return NULL;

	block = mem_alloc(size);
	if (!block)
		return NULL;

	sys_memset(block, 0, size);
	return block;
}