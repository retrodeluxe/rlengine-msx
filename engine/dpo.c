/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2019 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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
#include "dpo.h"
#include "msx.h"
#include "mem.h"
#include "list.h"
#include "ascii8.h"

#pragma CODE_PAGE 2

/*
 * Dynamically allocated animators
 */
Animator *dpo_animators;

/*
 * List of display objects
 */
List dpo_display_list;

/*
 * Auxiliary variables for dpo iteration
 */
static List *elem, *elem2;
static DisplayObject *dpo;
static Animator *anim;
static uint8_t page;

/**
 * Initializes the display list
 */
void dpo_init()
{
   INIT_LIST_HEAD(&dpo_display_list);
}

/**
 * Allocates a new DisplayObject
 */
DisplayObject *dpo_new()
{
  return mem_alloc(sizeof(DisplayObject));
}

/**
 * Does garbage collection of DisplayObjects part of the display list
 */
void dpo_clear()
{
  list_for_each(elem, &dpo_display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    mem_free(dpo);
  }
}

/**
 * Adds a DisplayObject to the display list
 *
 * :param dpo: DisplayObject to be added
 */
void dpo_display_list_add(DisplayObject *dpo)
{
  INIT_LIST_HEAD(&dpo->list);
  INIT_LIST_HEAD(&dpo->animator_list);
  list_add(&dpo->list, &dpo_display_list);
}

/**
 * Allocates memory for animators
 *
 * @param n_animators
 */
void dpo_init_animators(uint8_t n_animators)
{
  dpo_animators = mem_calloc(n_animators, sizeof(Animator));
}

/**
 * Adds an animator to a Display Object
 *
 * :param dpo: the DisplayObject
 * :param animidx: the animator index
 */
void dpo_add_animator(DisplayObject *dpo, uint8_t animidx)
{
  list_add(&dpo_animators[animidx].list, &dpo->animator_list);
}

/**
 * Shows on a screen buffer all DisplayObjects in the display list
 *
 */
void dpo_show_all(uint8_t *scr_buffer) __nonbanked
{
  list_for_each(elem, &dpo_display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    if (dpo->type == DISP_OBJECT_SPRITE && dpo->visible) {
      spr_show(dpo->spr);
    } else if (dpo->type == DISP_OBJECT_TILE && dpo->visible) {
      tile_object_show(dpo->tob, scr_buffer, false);
    }
  }
}

/**
 * Run animations for all DisplayObjects in the display list
 */
void dpo_animate_all() __nonbanked
{
  list_for_each(elem, &dpo_display_list) {
    dpo = list_entry(elem, DisplayObject, list);
    list_for_each(elem2, &dpo->animator_list) {
      anim = list_entry(elem2, Animator, list);
      page = ascii8_set_code(anim->page);
      anim->run(dpo);
      ascii8_restore_code(page);
    }
  }
}