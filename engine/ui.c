/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2021 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

#include "ui.h"
#include "tile.h"
#include "list.h"
#include "log.h"
#include "vdp.h"
#include "font.h"
#include "sys.h"
#include "sprite.h"

#pragma CODE_PAGE 2

int16_t ui_mouse_x, ui_mouse_y;
uint8_t ui_mouse_a, ui_mouse_b;
UiKeyCode ui_key;
UiWidget *w;
SpriteDef ui_mouse;

#define MAX_WIDGETS 80

UiWidget *widgets[MAX_WIDGETS];
uint8_t nwidgets;

uint8_t ui_hashmap[768];

const UiKeyCode kbdmatrix[8][8] = {
  {KEY_7,  KEY_6,  KEY_5,  KEY_4,  KEY_3,    KEY_2,   KEY_1, KEY_0},
  {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE,   KEY_EQU,  KEY_MIN, KEY_9, KEY_8},
  {KEY_B,  KEY_A,  KEY_NONE,  KEY_NONE, KEY_NONE,  KEY_NONE,  KEY_NONE, KEY_NONE },
  {KEY_J,  KEY_I,  KEY_H,  KEY_G,  KEY_F,    KEY_E,   KEY_D, KEY_C},
  {KEY_R,  KEY_Q,  KEY_P,  KEY_O,  KEY_N,    KEY_M,   KEY_L, KEY_K},
  {KEY_Z,  KEY_Y,  KEY_X,  KEY_W,  KEY_V,    KEY_U,   KEY_T, KEY_S},
  {KEY_F3, KEY_F2, KEY_F1, KEY_NONE,   KEY_CAPS, KEY_GRP, KEY_CTRL, KEY_SHIFT},
  {KEY_RET, KEY_SEL, KEY_BS, KEY_STOP, KEY_TAB, KEY_ESC, KEY_F5, KEY_F4},
  {KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT, KEY_DEL, KEY_INS, KEY_HOME, KEY_SPC}
};

/**
 * Initializes data structures related to widgets
 */
void ui_init(FontSet *fs, TileSet *ts, uint8_t mouse_ptrn_id) {
   tile_set_to_vram(ts, 1);

   sys_memset(widgets, 0, MAX_WIDGETS * 2);
   sys_memset(ui_hashmap, 255, 768);
   nwidgets = 0;

   ui_mouse_x = 128;
   ui_mouse_y = 107;

   spr_init_sprite(&ui_mouse, mouse_ptrn_id);
   spr_set_pos(&ui_mouse, ui_mouse_x, ui_mouse_y);
   spr_show(&ui_mouse);
}

/**
 * Adds a widget to the display list
 */
void ui_register_widget(UiWidget *widget) {
  if (nwidgets < MAX_WIDGETS) {
    widget->index = nwidgets;
    widgets[nwidgets++] = widget;
  }
}

void ui_set_hashmap(uint16_t hash, uint8_t value) {
  ui_hashmap[hash] = value;
}

#define UI_TITLE_BASE 32
#define UI_LABEL_BASE 64
#define UI_BUTTON_BASE 0

static void draw_title(UiWidget *widget) {
  char c;
	uint16_t addr = VRAM_BASE_NAME + widget->ypos * 32 + widget->xpos;

  while ((c = *(widget->label++) ) != 0) {
		vdp_write(addr++, c + UI_TITLE_BASE);
	}
}

static void draw_label(UiWidget *widget) {
  char c;
	uint16_t addr = VRAM_BASE_NAME + widget->ypos * 32 + widget->xpos;

  while ((c = *(widget->label++)) != 0) {
    if (c >= CHR_A && c <= CHR_Z )
		  vdp_write(addr++, c + UI_LABEL_BASE);
    else if (c >= CHR_a && c<= CHR_z)
      vdp_write(addr++, c + UI_LABEL_BASE);
  }
}

static void draw_button(UiWidget *widget) {
  char c;
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint16_t addr = VRAM_BASE_NAME + hash;
  uint8_t *label = widget->label;
  uint8_t idx = widget->index;
  uint8_t base = 0;

  if (widget->state != 0)
    base = 32;

  vdp_write(addr++, 27 + 64 - base);
  ui_hashmap[hash++] = idx;
  while ((c = *label++ ) != 0) {
		vdp_write(addr++, c - base);
    ui_hashmap[hash++] = idx;
	}
  vdp_write(addr++, 28 + 64 - base);
  ui_hashmap[hash++] = idx;
}

static void draw_range(UiWidget *widget) {
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint16_t addr2, addr = VRAM_BASE_NAME + hash;
  uint8_t idx = widget->index;
  uint16_t n, v, d;
  uint8_t base1 = 14, base2 = 16;
  uint8_t ndigits = 0;

  n = widget->max;
  while (n != 0) {
    n = n / 10;
    ndigits++;
    vdp_write(addr++, 4);
    hash++;
  }
  addr2 = addr;
  v = widget->value;
  while (v != 0) {
    vdp_write(--addr, 4 + v % 10);
    v = v / 10;
  }
  if (widget->state == 1)
    base1++;
  if (widget->state == 2)
    base2++;

  vdp_write(addr2++, base1);
  ui_hashmap[hash++] = idx;
  vdp_write(addr2++, base2);
  ui_hashmap[hash++] = idx;
}

static void draw_switch(UiWidget *widget) {
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint16_t addr = VRAM_BASE_NAME + hash;
  uint8_t idx = widget->index;

  if (widget->state == 0) {
    vdp_write(addr++, 3);
    ui_hashmap[hash++] = idx;
    vdp_write(addr++, 2);
    ui_hashmap[hash++] = idx;
  } else {
    vdp_write(addr++, 1);
    ui_hashmap[hash++] = idx;
    vdp_write(addr++, 3);
    ui_hashmap[hash++] = idx;
  }
}

#define UI_PANEL_BASE 21
static void draw_panel(UiWidget *widget) {
  uint16_t addr = VRAM_BASE_NAME + widget->ypos * 32 + widget->xpos;
  uint16_t i, j;

  vdp_write(addr++, UI_PANEL_BASE);
  for (i = 0; i < widget->w - 2; i++)
    vdp_write(addr++, UI_PANEL_BASE + 1);
  vdp_write(addr++, UI_PANEL_BASE + 2);
  addr += 32 - widget->w;
  for (i = 1; i < widget->h - 2; i++) {
    vdp_write(addr++, UI_PANEL_BASE + 6);
    for (j = 0; j < widget->w - 2; j++)
      vdp_write(addr++, UI_PANEL_BASE + 7);
    vdp_write(addr++, UI_PANEL_BASE + 8);
    addr += 32 - widget->w;
  }
  vdp_write(addr++, UI_PANEL_BASE + 3);
  for (i = 0; i < widget->w - 2; i++)
    vdp_write(addr++, UI_PANEL_BASE + 4);
  vdp_write(addr++, UI_PANEL_BASE + 5);
}

void ui_draw() {
  uint8_t i;
  for (i = 0; i < nwidgets; i++) {
    w = widgets[i];
    switch (w->type) {
      case WIDGET_TITLE:
        draw_title(w);
        break;
      case WIDGET_LABEL:
        draw_label(w);
        break;
      case WIDGET_BUTTON:
        draw_button(w);
        break;
      case WIDGET_SWITCH:
        draw_switch(w);
        break;
      case WIDGET_RANGE:
        draw_range(w);
        break;
      case WIDGET_PANEL:
        draw_panel(w);
        break;
      case WIDGET_CUSTOM:
        w->on_draw(w);
        break;
    }
  }
}

/**
 * this one is static
 */
void ui_handle_event(UiEvent *event) {
  uint16_t hash = (event->y / 8) * 32 + event->x / 8;
  uint8_t idx = ui_hashmap[hash];

  UiWidget *w;

  // TODO: dispatch key events

  if (idx != 255 &&
    (event->type == EVENT_MOUSE_BUTTON_DOWN
      || event->type == EVENT_MOUSE_BUTTON_UP)) {
    w = widgets[idx];
    switch(w->type) {
      case WIDGET_BUTTON:
        if (event->type == EVENT_MOUSE_BUTTON_DOWN) {
          w->state = 1;
          if (w->on_click != NULL)
            w->on_click();
        } else {
          w->state = 0;
        }
        draw_button(w);
        break;
      case WIDGET_SWITCH:
        if (event->type == EVENT_MOUSE_BUTTON_DOWN) {
          if(w->state == 0)
            w->state = 1;
          else
            w->state = 0;
          draw_switch(w);
          if (w->on_click != NULL)
            w->on_click();
        }
        break;
      case WIDGET_RANGE:
        if (event->type == EVENT_MOUSE_BUTTON_DOWN) {
          if(w->state == 0) {
            /* press, increase or decrease */
            if (ui_hashmap[hash - 1] == 255) {
              if (w->value > w->min) {
                w->value--;
                w->state = 1;
              }
            } else {
              if (w->value < w->max) {
                w->value++;
                w->state = 2;
              }
            }
          }
        } else {
          w->state = 0;
        }
        draw_range(w);
        if (w->on_click != NULL)
          w->on_click();
        break;
      case WIDGET_CUSTOM:
        if(w->on_handle_event != NULL)
          w->on_handle_event(w, event);
        break;
    }
  }
}

void ui_handle_events() {
  uint16_t offset;
  int8_t offset_x, offset_y;
  uint8_t mouse_a, mouse_b;
  uint8_t key_row, key_col, key_data;
  UiKeyCode key;
  UiEvent e;

  spr_refresh();
  offset = sys_get_mouse(12);
  offset_x = (int8_t)(offset & 0x00FF);
  offset_y = (int8_t)((offset & 0xFF00) >> 8);

  ui_mouse_x += offset_x;
  ui_mouse_y += offset_y;

  /* No need to adjust for screen size, this module is intended for GRP2 only */
  if (ui_mouse_x < 0) ui_mouse_x = 0;
  if (ui_mouse_x > 255) ui_mouse_x = 255;
  if (ui_mouse_y < 0) ui_mouse_y = 0;
  if (ui_mouse_y > 191) ui_mouse_y = 191;

  spr_set_pos(&ui_mouse, ui_mouse_x, ui_mouse_y);
  spr_update(&ui_mouse);

  mouse_a = sys_get_trigger(1);
  mouse_b = sys_get_trigger(3);

  /* raise event on mouse button raising/falling edges */
  if (ui_mouse_a != mouse_a) {
    ui_mouse_a = mouse_a;
    e.x = ui_mouse_x;
    e.y = ui_mouse_y;
    if (mouse_a == 0)
      e.type = EVENT_MOUSE_BUTTON_UP;
    else
      e.type = EVENT_MOUSE_BUTTON_DOWN;
    ui_handle_event(&e);
  }

  /* scan a single key from keyboard matrix */
  /* TODO: scan modifiers separately SHIFT+ CTRL+ */
  for (key_row = 0; key_row < 9; key_row++) {
    key_data = sys_get_keyrow(key_row);
    key = KEY_NONE;
    if (key_data != 255) {
      key_col = 7;
      key_data = (uint8_t)(~key_data);
      while ((key_data >>= 1) != 0) {
        key_col--;
      }
      key = kbdmatrix[key_row][key_col];
      break;
    }
  }

  if (ui_key != key) {
    ui_key = key;
    if (key == KEY_NONE)
      e.type = EVENT_KEYUP;
    else
      e.type = EVENT_KEYDOWN;
    e.key = key;
    ui_handle_event(&e);
  }
}
