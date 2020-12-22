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

#include "log.h"
#include "msx.h"
#include "sprite.h"
#include "sys.h"
#include "vdp.h"

#pragma CODE_PAGE 2

// vram sprite attribute allocation table
uint8_t spr_attr_valloc[MAX_SPR_ATTR];

// vram sprite pattern allocation table
uint8_t spr_patt_valloc[MAX_SPR_PTRN];

// spr pattern sets
SpritePattern spr_pattern[SPR_PATRN_MAX];

// spr pattern attr
VdpSpriteAttr spr_attr[MAX_SPR_ATTR];

/**
 * Initialize Sprite module
 *  Calling this function clears all defined patters and frees allocations.
 */
void spr_init(void) {
  spr_clear();
  sys_memset(spr_pattern, 0, sizeof(SpritePattern) * SPR_PATRN_MAX);
}

/**
 * Apply changes made to SpriteDefs into VRAM

 * This function should be called after running :c:func:`sys_wait_vsync` and is
 * required for changes due to :c:func:`spr_update` to be visible on screen.
 */
bool flip;
void spr_refresh(void) {
  uint8_t _5th_sprite, i, ct;

  /**
   * This fuction does 5th sprite detection and interleaving
   *
   * The first 3 attributes are reserved for the main character sprite
   * and won't be interleaved to avoid flickering on that sprite. The rest of
   * the sprite attributes are sent in reverse order to ensure the 5th sprite
   * is visible. This works well for 5th sprites in a row, but not for more
   * which requires a more expensive algorithm.
   */
  _5th_sprite = vdp_5th_sprite();
  if(_5th_sprite && flip) {
    for (i = MAX_SPR_ATTR - 1, ct = 3; i > 2; i--) {
        if (spr_attr[i].y != SPR_OFF)
          vdp_memcpy(VRAM_BASE_SATR + sizeof(VdpSpriteAttr) * ct++,
            (uint8_t *)&spr_attr[i],
            sizeof(VdpSpriteAttr));
    }
    vdp_memcpy(VRAM_BASE_SATR, (uint8_t *)&spr_attr,
            sizeof(VdpSpriteAttr) * 3);
    flip = false;
  } else {
    vdp_memcpy(VRAM_BASE_SATR, (uint8_t *)&spr_attr,
            sizeof(VdpSpriteAttr) * MAX_SPR_ATTR);
    flip = true;
  }
}

/**
 * Clear all sprites from VRAM and frees all allocations. Preserves
 * defined patterns.
 */
void spr_clear(void) {
  uint8_t i;

  // FIXME: why this dependency here?
  vdp_init_hw_sprites(SPR_SIZE_16, SPR_ZOOM_OFF);

  /* entirely disable sprites by setting y=SPR_OFF */
  sys_memset(spr_attr, SPR_OFF, sizeof(VdpSpriteAttr) * MAX_SPR_ATTR);
  sys_memset(spr_attr_valloc, 1, MAX_SPR_ATTR);
  sys_memset(spr_patt_valloc, 1, MAX_SPR_PTRN);

  // free pattern sets
  for (i = 0; i < SPR_PATRN_MAX; i++)
    spr_vfree_pattern_set(i);
}

/**
 * Initializes a SpriteDef structure
 *
 * :param sp: a SpriteDef object
 * :param patrn_idx: sprite pattern set index (0-47)
 */
void spr_init_sprite(SpriteDef *sp, uint8_t patrn_idx) {
  assert(patrn_idx < SPR_PATRN_MAX, "Max pattern index should be below 48");

  sp->pattern_set = &spr_pattern[patrn_idx];
  sp->cur_anim_step = 0;
  sp->cur_state = 0;
  sp->anim_ctr_treshold = 5;
  sp->anim_ctr = 0;
  spr_set_plane_colors(sp, spr_pattern[patrn_idx].colors);
}

/**
 * Allocates VRAM and transfers a SpritePattern making the Sprite ready
 * for visualization.
 *
 * :param patrn_idx: sprite pattern set index (0-47)
 * :returns: true if sucess, false is the pattern could not be allocated
 */
bool spr_valloc_pattern_set(uint8_t patrn_idx) {
  uint16_t npat;
  uint8_t i, idx, size, f = 0;
  uint8_t n_steps = 0;

  assert(patrn_idx < SPR_PATRN_MAX, "Max pattern index should be below 48");

  SpritePattern *ps = &spr_pattern[patrn_idx];

  if (ps->allocated)
    return true;

  for (i = 0; i < ps->n_states; i++) {
    n_steps += ps->state_steps[i];
  }
  ps->n_steps = n_steps;

  size = ps->size;
  if (ps->size == SPR_SIZE_32x16)
    size = 8;
  if (ps->size == SPR_SIZE_32x32)
    size = 16;

  npat = ps->n_planes * ps->n_steps * size;

  for (i = 0; i < MAX_SPR_PTRN - 1; i++) {
    f = f * spr_patt_valloc[i] + spr_patt_valloc[i];
    if (f == npat) {
      idx = i - npat + 1;
      sys_memset(&spr_patt_valloc[idx], 0, npat);
      vdp_memcpy(VRAM_BASE_SPAT + idx * 8, ps->patterns, npat * 8);
      sys_memcpy(ps->colors2, ps->colors, npat / 4);
      ps->pidx = idx;
      ps->allocated = true;
      return true;
    }
  }
  return false;
}

/**
 * Frees the VRAM used by the specified SpritePattern
 *
 * :param patrn_idx: sprite pattern set index (0-47)
 */
void spr_vfree_pattern_set(uint8_t patrn_idx) {
  uint8_t npat, size;

  assert(patrn_idx < SPR_PATRN_MAX, "Max pattern index should be below 48");
  SpritePattern *ps = &spr_pattern[patrn_idx];

  size = ps->size;
  if (ps->size == SPR_SIZE_32x16)
    size = 8;
  if (ps->size == SPR_SIZE_32x32)
    size = 16;

  npat = ps->n_planes * ps->n_steps * size;
  sys_memset(&spr_patt_valloc[ps->pidx], 1, npat);
  ps->allocated = false;
}


static void spr_calc_patterns(SpriteDef *sp) __nonbanked {
  uint8_t i, cf, base = 0, base2, frame, np, sz, as;

  SpritePattern *ps = sp->pattern_set;
  for (i = 0; i < sp->cur_state; i++) {
    base += ps->state_steps[i];
  }

  np = ps->n_planes;
  sz = ps->size;
  as = sp->cur_anim_step;
  cf = (base + as) * np;

  switch (sz) {
  case SPR_SIZE_16x16:
    base *= (sz * np);
    base += as * (sz * np); // current frame
    for (i = 0; i < np; i++) {
      SET_PLANE_PTRN(sp, i, (ps->colors2)[cf + i], base + i * sz);
    }
    break;
  case SPR_SIZE_16x32:
    base *= (SPR_SIZE_16x16 * np);
    base += as * (SPR_SIZE_16x16 * np); // current frame
    base2 = base + np * ps->n_steps * SPR_SIZE_16x16;
    for (i = 0; i < np; i++) {
      SET_PLANE_PTRN(sp, i, (ps->colors2)[cf + i], base + i * SPR_SIZE_16x16);
      SET_PLANE_PTRN(sp, i + np, (ps->colors2)[cf + i],
        base2 + i * SPR_SIZE_16x16);
    }
    break;
  case SPR_SIZE_32x16:
    base *= (SPR_SIZE_16x32 * np);
    base += as * SPR_SIZE_16x32 * np; // current frame
    base2 = base + 4;
    for (i = 0; i < np; i++) {
      SET_PLANE_PTRN(sp, i, (ps->colors2)[cf], base + i * SPR_SIZE_16x32);
      SET_PLANE_PTRN(sp, i + np, (ps->colors2)[cf],
        base2 + i * SPR_SIZE_16x32);
    }
    break;
  case SPR_SIZE_32x32:
    base *= SPR_SIZE_16x32;
    base += as * SPR_SIZE_16x32; // current frame
    base2 = base + ps->n_steps * SPR_SIZE_16x32;
    SET_PLANE_PTRN(sp, 0, (ps->colors2)[cf], base);
    SET_PLANE_PTRN(sp, 1, (ps->colors2)[cf], base + 4);
    SET_PLANE_PTRN(sp, 2, (ps->colors2)[cf], base2);
    SET_PLANE_PTRN(sp, 3, (ps->colors2)[cf], base2 + 4);
    break;
  }
}

/**
 * Updates a SpriteDef
 *  This function must be called after
 *  It is necessary to call :c:func:`spr_refresh` for the changes to be
 *  transferred to VRAM and be visible on screen.
 *
 * :param sp: a SpriteDef object
 */
void spr_update(SpriteDef *sp) __nonbanked {
  uint8_t i, np, sz;

  np = sp->pattern_set->n_planes;
  sz = sp->pattern_set->size;

  spr_calc_patterns(sp);
  for (i = 0; i < np; i++) {
    sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i], (uint8_t *)&sp->planes[i], 4);
    if (sz == SPR_SIZE_16x32 ||
        sz == SPR_SIZE_32x16) {
      sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + np],
                 (uint8_t *)&sp->planes[i + np], 4);
    } else if (sz == SPR_SIZE_32x32) {
      sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 1],
                 (uint8_t *)&sp->planes[1], 4);
      sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 2],
                 (uint8_t *)&sp->planes[2], 4);
      sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 3],
                 (uint8_t *)&sp->planes[3], 4);
    }
  }
}

/**
 * Allocates a SpriteDef in VRAM
 *  this function
 *
 * :param sp: a SpriteDef object
 * :return: true on sucess, false if the Sprite could not be allocated
 */
bool spr_show(SpriteDef *sp) __nonbanked {
  uint8_t i, sz, idx = 7, n, f = 0;
  n = sp->pattern_set->n_planes;
  sz = sp->pattern_set->size;

  if (sz == SPR_SIZE_16x32 ||
      sz == SPR_SIZE_32x16)
    n = n * 2;
  else if (sz == SPR_SIZE_32x32)
    n = n * 4;
  for (i = 0; i < MAX_SPR_ATTR - 1; i++) {
    f = f * spr_attr_valloc[i] + spr_attr_valloc[i];
    if (f == n) {
      idx = i - n + 1;
      sys_memset(&spr_attr_valloc[idx], 0, n);
      sp->aidx = idx;
      spr_update(sp);
      return true;
    }
  }
  return false;
}

/**
 * Frees a SpriteDef from VRAM
 *  calling this function removes the Sprite from screen.
 *
 *  It is necessary to call :c:func:`spr_refresh` for the changes to be
 *  transferred to VRAM and be visible on screen.
 *
 * :param sp: SpriteDef object
 */
void spr_hide(SpriteDef *sp) __nonbanked {
  uint8_t n, idx, sz;
  VdpSpriteAttr null_spr;

  sz = sp->pattern_set->size;
  n = sp->pattern_set->n_planes;
  if (sz == SPR_SIZE_16x32
    || sz == SPR_SIZE_32x16)
    n = n * 2;
  else if (sz == SPR_SIZE_32x32)
    n = n * 4;
  idx = sp->aidx;
  sys_memset(&spr_attr_valloc[idx], 1, n);

  // FIXME: this should actually use SPR_OFF and re-arrange the
  //        attributes so that only active sprites remain

  /* set sprite outside screen using EC bit */
  null_spr.y = 193;
  null_spr.x = 0;
  null_spr.pattern = 0;
  null_spr.color = 128; // EC bit

  // FIXME: still wrong handling of multiple planes
  sys_memcpy((uint8_t *)&spr_attr[sp->aidx], (uint8_t *)&null_spr,
             sizeof(VdpSpriteAttr));
  if (sz == SPR_SIZE_16x32 ||
      sz == SPR_SIZE_32x16) {
    sys_memcpy((uint8_t *)&spr_attr[sp->aidx + 1], (uint8_t *)&null_spr,
               sizeof(VdpSpriteAttr));
  } else if (sz == SPR_SIZE_32x32) {
    // TODO
  }
}

/**
 * Set a Sprite position on screen taking into account off-screen coordinates
 *
 *  It is necessary to call :c:func:`spr_refresh` for the changes to be
 *  transferred to VRAM and be visible on screen.
 *
 * :param sp: a SpriteDef object
 * :param xp: x screen coordinate (-32 to 256)
 * :param yp: y screen coordinate (-32 to 192)
 */
void spr_set_pos(SpriteDef *sp, int16_t xp, int16_t yp) __nonbanked {
  uint8_t i, x, x2, y, np, sz, ec = 0, ec2 = 0;

  np = sp->pattern_set->n_planes;
  sz = sp->pattern_set->size;

  if (yp > -33 && yp < 0)
    y = (int8_t)yp;
  else if (yp == 0)
    y = 0xFF;
  else if (yp > 0 && yp < 193)
    y = yp - 1;

  if (xp < 0) {
    x = xp + 32;
    ec = 128;
  } else if (xp >= 0 && xp < 256) {
    x = xp;
  }

  // wide sprites need additional off-screen adjustment
  if (sz == SPR_SIZE_32x16 || sz == SPR_SIZE_32x32) {
    if (xp < -16) {
        x2 = xp + 32;
        ec2 = 128;
    } else if (xp >= 239) {
        x2 = -16;
        ec2 = 128;
    } else if (xp >= -16 && xp < 239) {
        x2 = xp;
    }
  }

  for (i = 0; i < np; i++) {
    SET_PLANE_ATTR(sp, i, x, y, ec);
    if (sz == SPR_SIZE_16x32) {
      SET_PLANE_ATTR(sp, i + np, x, y + 16, ec);
    } else if (sz == SPR_SIZE_32x16) {
      SET_PLANE_ATTR(sp, i + np, x2 + 16, y, ec2);
    } else if (sz == SPR_SIZE_32x32) {
      SET_PLANE_ATTR(sp, 1, x2 + 16, y, ec2);
      SET_PLANE_ATTR(sp, 2, x, y + 16, ec);
      SET_PLANE_ATTR(sp, 3, x2 + 16, y + 16, ec2);
    }
  }
}

void spr_set_plane_colors(SpriteDef *sp, uint8_t *colors) __nonbanked {
  uint8_t i, np, sz;
  sz = sp->pattern_set->size;
  np = sp->pattern_set->n_planes;
  for (i = 0; i < np; i++) {
    (sp->planes[i]).color = colors[i];
    if (sz == SPR_SIZE_16x32 ||
        sz == SPR_SIZE_32x16) {
      (sp->planes[i + np]).color = colors[i];
    } else if (sz == SPR_SIZE_32x32) {
      (sp->planes[1]).color = colors[i];
      (sp->planes[2]).color = colors[i];
      (sp->planes[3]).color = colors[i];
    }
  }
}

/**
 * Updates a SpriteDef state and animation frame based on the direction
 * of movement, for a simple case of 2 or 4 states.
 *
 * :param sp: a SpriteDef object
 * :param dx: delta X
 * :param dy: delta Y
 */
void spr_animate(SpriteDef *sp, int8_t dx, int8_t dy) __nonbanked {
  uint8_t old_dir, x, y;
  SpritePattern *ps = sp->pattern_set;

  old_dir = sp->cur_state;

  /* update state based on direction of movement */
  if (sp->pattern_set->n_states < 2) {
    // keep current state, no changes
  } else if (sp->pattern_set->n_states < 3) {

    if (dx > 0) {
      sp->cur_state = SPR_STATE_RIGHT;
    } else if (dx < 0) {
      sp->cur_state = SPR_STATE_LEFT;
    }

  } else if (sp->pattern_set->n_states < 5) {

    if (dx > 0) {
      sp->cur_state = SPR_STATE_RIGHT;
    } else if (dx < 0) {
      sp->cur_state = SPR_STATE_LEFT;
    }
    if (dy > 0) {
      sp->cur_state = SPR_STATE_DOWN;
    } else if (dy < 0) {
      sp->cur_state = SPR_STATE_UP;
    }

  } else {
    log_e("Only 2 or 4 states supported\n");
  }

  /* update animation frame */
  if (old_dir == sp->cur_state) {
    sp->anim_ctr++;
    if (sp->anim_ctr > sp->anim_ctr_treshold) {
      sp->cur_anim_step++;
      sp->anim_ctr = 0;
    }
  } else {
    sp->cur_anim_step = 0;
  }

  if (sp->cur_anim_step > ps->state_steps[sp->cur_state] - 1)
    sp->cur_anim_step = 0;
}
