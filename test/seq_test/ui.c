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

#pragma CODE_PAGE 3

#define MAX_WIDGETS 50

int16_t ui_mouse_x, ui_mouse_y;
uint8_t ui_mouse_a, ui_mouse_b;
UiKeyCode ui_key;
UiWidget *w;
SpriteDef ui_mouse;
Font *ui_font;
UiWidget widgets[MAX_WIDGETS];
uint8_t nwidgets;
uint8_t ui_hashmap[768];
uint8_t ui_keymap[64];
uint8_t *ui_buffer;

#define UI_TITLE_BASE 32
#define UI_LABEL_BASE 64
#define UI_BUTTON_BASE 0
#define UI_PANEL_BASE 21

const UiKeyCode kbdmatrix[8][8] = {
  {KEY_7, KEY_6, KEY_5, KEY_4, KEY_3, KEY_2, KEY_1, KEY_0},
  {KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_EQU, KEY_MIN, KEY_9, KEY_8},
  {KEY_B, KEY_A, KEY_NONE, KEY_NONE, KEY_NONE, KEY_COMMA, KEY_NONE, KEY_NONE},
  {KEY_J, KEY_I, KEY_H, KEY_G, KEY_F, KEY_E, KEY_D, KEY_C},
  {KEY_R, KEY_Q, KEY_P, KEY_O, KEY_N, KEY_M, KEY_L, KEY_K},
  {KEY_Z, KEY_Y, KEY_X, KEY_W, KEY_V, KEY_U, KEY_T, KEY_S},
  {KEY_F3, KEY_F2, KEY_F1, KEY_NONE, KEY_CAPS, KEY_GRP, KEY_CTRL, KEY_SHIFT},
  {KEY_RET, KEY_SEL, KEY_BS, KEY_STOP, KEY_TAB, KEY_ESC, KEY_F5, KEY_F4},
  {KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT, KEY_DEL, KEY_INS, KEY_HOME, KEY_SPC}
};

/**
 * Initializes data structures related to widgets
 */
void ui_init(Font *font, uint8_t *scr_buffer, uint8_t mouse_ptrn_id) {
   sys_memset(widgets, 0, MAX_WIDGETS * sizeof(UiWidget));
   sys_memset(ui_hashmap, 255, 768);
   nwidgets = 0;

   ui_mouse_x = 128;
   ui_mouse_y = 107;
   ui_key = KEY_NONE;

   font_to_vram(font, 1);
   ui_font = font;
   ui_buffer = scr_buffer;

   spr_init_sprite(&ui_mouse, mouse_ptrn_id);
   spr_set_pos(&ui_mouse, ui_mouse_x, ui_mouse_y);
   spr_show(&ui_mouse);
}

/**
 * Adds a widget to the display list
 */
UiWidget* ui_new_widget(UiWidgetType type, TileSet *ts, TileBank banks) {
  rle_result res;
  if (nwidgets < MAX_WIDGETS) {
    if (ts) {
      if (banks == ALLBANKS)
        res = tile_set_valloc(ts);
      else
        res = tile_set_valloc_bank(ts, banks);
      if (res == RLE_COULD_NOT_ALLOCATE_VRAM) {
        log_e("ui: could not allocate tileset %d\n", res);
        return NULL;
      }
    }
    widgets[nwidgets].index = nwidgets;
    widgets[nwidgets].type = type;
    widgets[nwidgets].tileset = ts;
    widgets[nwidgets].bank = banks;
    return &widgets[nwidgets++];
  }
  return NULL;
}

void ui_set_keybinding(UiWidget *widget, UiKeyCode keycode) {
  ui_keymap[keycode] = widget->index;
}

void ui_set_hashmap(uint16_t hash, uint8_t value) {
  ui_hashmap[hash] = value;
}

static void draw_label(UiWidget *widget) {
  FontSet fs;
  fs.upper = ui_font;
  font_printf(&fs, widget->xpos, widget->ypos, ui_buffer, widget->label);
}

static void draw_icon_button(UiWidget *widget) {
  char c;
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint8_t *addr = ui_buffer + hash;
  uint8_t *label = widget->label;
  uint8_t idx = widget->index;
  uint8_t base = 0, base2 = widget->tileset->pidx;

  if (widget->state == 1) {
    base = 32;
    base2++;
  }
  if (widget->state == WIDGET_HOLD) {
    *addr++ = 28 + 64 - base + 1;
  } else {
    *addr++ = 27 + 64 - base;
  }
  ui_hashmap[hash++] = idx;

  // icon
  *addr++ = base2;
  ui_hashmap[hash++] = idx;

  if (label) {
    while ((c = *label++ ) != 0) {
      *addr++ = c - base;
      ui_hashmap[hash++] = idx;
    }
  }

  *addr++ = 28 + 64 - base;
  ui_hashmap[hash++] = idx;
}

static void draw_button(UiWidget *widget) {
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint8_t *addr = ui_buffer + hash;
  uint8_t *label = widget->label;
  uint8_t idx = widget->index;
  uint8_t base = widget->tileset->pidx;
  char c;

  if (widget->state == 0)
    base += 29; // length of button tileset

  *addr++ = 26 + base;
  ui_hashmap[hash++] = idx;
  while ((c = *label++ ) != 0) {
		*addr++ = c - 65 + base;
    ui_hashmap[hash++] = idx;
	}
  *addr++ = 27 + base;
  ui_hashmap[hash++] = idx;
}

static void draw_range(UiWidget *widget) {
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint8_t *addr2, *addr = ui_buffer + hash;
  uint8_t idx = widget->index;
  uint16_t n, v, d;
  uint8_t base1 = 10, base2 = 12;
  uint8_t ndigits = 0;
  uint8_t base = widget->tileset->pidx;

  n = widget->max;
  while (n != 0) {
    n = n / 10;
    ndigits++;
    *addr++ = base;
    hash++;
  }
  addr2 = addr;
  v = widget->value;
  while (v != 0) {
    *(--addr) =  base + v % 10;
    v = v / 10;
  }

  if (widget->on_click) {
    if (widget->state == 1)
      base1++;
    if (widget->state == 2)
      base2++;

    *addr2++ = base + base1;
    ui_hashmap[hash++] = idx;
    *addr2++ = base + base2;
    ui_hashmap[hash++] = idx;
  }
}

static void draw_switch(UiWidget *widget) {
  uint16_t hash = widget->ypos * 32 + widget->xpos;
  uint8_t *addr = ui_buffer + hash;
  uint8_t base = widget->tileset->pidx;
  uint8_t idx = widget->index;

  if (widget->state == 0) {
    *addr++ = base + 2;
    ui_hashmap[hash++] = idx;
    *addr++ = base + 1;
    ui_hashmap[hash++] = idx;
  } else {
    *addr++ = base;
    ui_hashmap[hash++] = idx;
    *addr++ = base + 2;
    ui_hashmap[hash++] = idx;
  }
}

static void draw_panel(UiWidget *widget) {
  uint8_t *addr = ui_buffer + widget->ypos * 32 + widget->xpos;
  uint16_t base = widget->tileset->pidx;
  uint16_t i, j;

  *addr++ = base;
  for (i = 0; i < widget->w - 2; i++)
    *addr++ = base + 1;
  *addr++ = base + 2;
  addr += 32 - widget->w;
  for (i = 1; i < widget->h - 2; i++) {
    *addr++ = base + 6;
    for (j = 0; j < widget->w - 2; j++)
      *addr++ = base + 7;
    *addr++ = base + 8;
    addr += 32 - widget->w;
  }
  *addr++ = base + 3;
  for (i = 0; i < widget->w - 2; i++)
    *addr++ = base + 4;
  *addr++ = base + 5;
}

void ui_draw() {
  uint8_t i;
  for (i = 0; i < nwidgets; i++) {
    w = &widgets[i];
    switch (w->type) {
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
      case WIDGET_ICON_BUTTON:
        draw_icon_button(w);
        break;
      case WIDGET_ICON:
        break;
      case WIDGET_CUSTOM:
        w->on_draw(w);
        break;
    }
  }
}

static void ui_handle_event(UiEvent *event) {
  uint16_t hash;
  uint8_t idx;
  UiWidget *w;

  if (event->type == EVENT_KEYDOWN
    || event->type == EVENT_KEYUP) {
    idx = ui_keymap[event->key];
    if (idx != 255) {
      w = &widgets[idx];
      switch (w->type) {
        case WIDGET_BUTTON:
          break;
        case WIDGET_CUSTOM:
          if (w->on_key_event)
            w->on_key_event(w, event);
          break;
      }
    }
  }

  if (event->type == EVENT_MOUSE_BUTTON_DOWN
      || event->type == EVENT_MOUSE_BUTTON_UP
      || event->type == EVENT_MOUSE_DRAG) {
    hash = (event->y / 8) * 32 + event->x / 8;
    idx = ui_hashmap[hash];
    if (idx != 255) {
      w = &widgets[idx];
      switch(w->type) {
        case WIDGET_ICON_BUTTON:
          if (event->type == EVENT_MOUSE_BUTTON_DOWN) {
            w->state = 1;
            if (w->on_click)
              w->on_click(w);
          } else {
            //if (w->state == 1)
            w->state = 2;
            //else
            //  w->state = 0;
          }
          draw_icon_button(w);
          break;
        case WIDGET_BUTTON:
          if (event->type == EVENT_MOUSE_BUTTON_DOWN) {
            w->state = 1;
            if (w->on_click)
              w->on_click(w);
          } else {
            if (w->state == 1)
              w->state = 2;
            else
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
            if (w->on_click)
              w->on_click(w);
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
          } else if (event->type == EVENT_MOUSE_BUTTON_UP){
            w->state = 0;
          }
          draw_range(w);
          if (w->on_click)
            w->on_click(w);
          break;
        case WIDGET_CUSTOM:
          if(w->on_mouse_event)
            w->on_mouse_event(w, event);
          break;
      }
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
    if (mouse_a == 0) {
      e.type = EVENT_MOUSE_BUTTON_UP;
    } else {
      e.type = EVENT_MOUSE_BUTTON_DOWN;
    }
    ui_handle_event(&e);
  }

  /* send drag events */
  if (ui_mouse_a) {
    e.type = EVENT_MOUSE_DRAG;
    e.dx = offset_x;
    e.dy = offset_y;
    e.x = ui_mouse_x;
    e.y = ui_mouse_y;
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
      // FIXME: handle 2 keys in the same row
      while (key_data >>= 1) {
        key_col--;
      }
      key = kbdmatrix[key_row][key_col];
      break;
    }
  }

  // handling multiple keys
  if (ui_key != key) {
    if (key == KEY_NONE) {
      e.type = EVENT_KEYUP;
      e.key = ui_key;
    } else {
      /* send key up for previously pressed key */
      if (ui_key != KEY_NONE) {
        e.type = EVENT_KEYUP;
        e.key = ui_key;
        ui_handle_event(&e);
      }
      e.type = EVENT_KEYDOWN;
      e.key = key;
    }
    ui_key = key;
    ui_handle_event(&e);
  }
}
