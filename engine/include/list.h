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

#ifndef _RDL_LIST_H_
#define _RDL_LIST_H_

/* simple linked list implementation based on the one from linux */

#include <stddef.h>

#define container_of(_ptr, _type, _member)                                     \
  ((_type *)((char *)_ptr - offsetof(_type, _member)))

/**
 * Defines a List Object
 */
typedef struct List List;

/**
 * Contents of a List
 */
struct List {
  /** pointer to the next element */
  List *next;
  /** pointer to the previous element */
  List *prev;
};

/**
 * Get the current object pointed to by a list entry
 *
 * :param ptr:	the List entry pointer.
 * :param type:	the type of object embedded in.
 * :param member:	the name of the List within the struct.
 */
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * Iterate over a list
 *
 * :param pos:	the &List to use as a loop cursor.
 * :param head:	the head for your list.
 */
#define list_for_each(pos, head) for (pos = (head)->next; pos; pos = pos->next)

extern void INIT_LIST_HEAD(List *list) __nonbanked;
extern void list_add(List *new, List *head) __nonbanked;
extern void list_del(List *entry) __nonbanked;

#endif
