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
#ifndef _MSX_H_UI_
#define _MSX_H_UI_

#include "msx.h"
#include "tile.h"
#include "font.h"
#include "list.h"

typedef enum {
  WIDGET_LABEL,
  WIDGET_BUTTON,
  WIDGET_ICON_BUTTON,
  WIDGET_RANGE,
  WIDGET_SWITCH,
  WIDGET_ICON,
  WIDGET_PANEL,
  WIDGET_CUSTOM
} UiWidgetType;

typedef enum {
  EVENT_MOUSE_BUTTON_DOWN,
  EVENT_MOUSE_BUTTON_UP,
  EVENT_MOUSE_DRAG,
  EVENT_KEYDOWN,
  EVENT_KEYUP
} UiEventType;

typedef enum {
  WIDGET_IDLE,
  WIDGET_PRESSED,
  WIDGET_HOLD
} UiWidgetState;

typedef enum {
  KEY_NONE,
  KEY_9, KEY_8, KEY_7, KEY_6, KEY_5, KEY_4, KEY_3, KEY_2, KEY_1, KEY_0, KEY_EQU,
  KEY_MIN, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_LEFT, KEY_RIGHT, KEY_UP,
  KEY_DOWN, KEY_HOME, KEY_INS, KEY_DEL, KEY_SPC, KEY_CAPS, KEY_GRP, KEY_CTRL,
  KEY_SHIFT, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
  KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
  KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_RET, KEY_SEL, KEY_BS, KEY_STOP,
  KEY_TAB, KEY_ESC, KEY_COMMA
} UiKeyCode;

typedef struct UiEvent UiEvent;
struct UiEvent {
  UiEventType type;
  uint8_t x;
  uint8_t y;
  int8_t dx;
  int8_t dy;
  uint8_t key;
  uint8_t key_modifier;
};

typedef struct UiWidget UiWidget;
struct UiWidget {
  uint8_t index;
  UiWidgetType type;
  UiKeyCode keybinding;
  uint8_t xpos;
  uint8_t ypos;
  uint8_t w;
  uint8_t h;
  uint8_t enabled;
  uint8_t state;
  int16_t value;
  int16_t max;
  int16_t min;
  uint8_t tag;
  char *label;
  TileSet *tileset;
  TileBank bank;
  void (*on_draw)(UiWidget *widget);
  void (*on_mouse_event)(UiWidget *widget, UiEvent *event);
  void (*on_click)(UiWidget *widget);
  void (*on_key_event)(UiWidget *widget, UiEvent *event);
};

extern void ui_init(Font *font, uint8_t *scr_buf, uint8_t mouse_ptrn_id);
extern UiWidget* ui_new_widget(UiWidgetType type, TileSet *ts, TileBank banks);
extern void ui_set_keybinding(UiWidget *widget, UiKeyCode keycode);
extern void ui_handle_events();
extern void ui_set_hashmap(uint16_t hash, uint8_t value);
#endif
