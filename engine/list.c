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
#include "list.h"


/**
 * Initialize a List object
 *
 * :param list: pointer to the List object
 */
void INIT_LIST_HEAD(List *list) __nonbanked {
  list->next = 0;
  list->prev = 0;
}

/**
 * Add a new entry to a List after the specified head.
 *
 * :param new: new entry to be added
 * :param head: List head to add it after
 *
 */
void list_add(List *new, List *head) __nonbanked {
  List *next = head->next;
  List *prev = head;
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

/**
 * Deletes entry from a list
 *
 * :param entry: the element to delete from the list.
 */
void list_del(List *entry) __nonbanked {
  entry->next->prev = entry->prev;
  entry->prev->next = entry->next;
  entry->next = 0;
  entry->prev = 0;
}
