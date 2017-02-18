#ifndef _MSX_DISPLAY_LIST_H_
#define _MSX_DISPLAY_LIST_H_

#define DISPLAY_OBJECT_SPRITE 1
#define DISPLAY_OBJECT_GFX 2

struct display_object;

struct animator {
  void (*next)(struct display_object *obj);
};

struct display_object {
  int type;
  int state;
  uint8_t xpos;
  uint8_t ypos;
  uint8_t collision_state;
  struct spr_sprite_def *spr;
  struct animator *animator;
};

#endif
