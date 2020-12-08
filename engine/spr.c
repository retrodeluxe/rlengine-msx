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
uint8_t spr_attr_valloc[vdp_hw_max_sprites];

// vram sprite pattern allocation table
uint8_t spr_patt_valloc[vdp_hw_max_patterns];

// spr pattern sets
SpritePattern spr_pattern[SPR_PATRN_MAX];

// spr pattern attr
struct vdp_hw_sprite spr_attr[vdp_hw_max_sprites];

/**
 * spr_init: initialize vdp sprites and allocation tables
 */
void spr_init(void) {
  spr_clear();
  sys_memset(spr_pattern, 0, sizeof(SpritePattern) * SPR_PATRN_MAX);
}

void spr_refresh(void) {
  vdp_memcpy(vdp_base_spatr_grp1, (uint8_t *)&spr_attr,
             sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites);
}

void spr_clear(void) {
  uint8_t i;

  vdp_init_hw_sprites(SPR_SHOW_16x16, SPR_ZOOM_OFF);

  /** entirely disable sprites by setting y=208 **/
  sys_memset(spr_attr, 208, sizeof(struct vdp_hw_sprite) * vdp_hw_max_sprites);
  sys_memset(spr_attr_valloc, 1, vdp_hw_max_sprites);
  sys_memset(spr_patt_valloc, 1, vdp_hw_max_patterns);

  // free pattern sets
  for (i = 0; i < SPR_PATRN_MAX; i++)
    spr_vfree_pattern_set(i);
}

void spr_init_sprite(SpriteDef *sp, uint8_t patrn_idx) {
  sp->pattern_set = &spr_pattern[patrn_idx];
  sp->cur_anim_step = 0;
  sp->cur_state = 0;
  sp->anim_ctr_treshold = 5;
  sp->anim_ctr = 0;
  spr_set_plane_colors(sp, spr_pattern[patrn_idx].colors);
}

/**
 * spr_valloc_pattern_set:
 *		finds a gap to allocate a pattern set
 */
uint8_t spr_valloc_pattern_set(uint8_t patrn_idx) {
  uint16_t npat;
  uint8_t i, idx, size, f = 0;
  uint8_t n_steps = 0;

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

  for (i = 0; i < vdp_hw_max_patterns - 1; i++) {
    f = f * spr_patt_valloc[i] + spr_patt_valloc[i];
    if (f == npat) {
      idx = i - npat + 1;
      sys_memset(&spr_patt_valloc[idx], 0, npat);
      vdp_memcpy(vdp_base_sppat_grp1 + idx * 8, ps->patterns, npat * 8);
      sys_memcpy(ps->colors2, ps->colors, npat / 4);
      ps->pidx = idx;
      ps->allocated = true;
      return true;
    }
  }
  return false;
}

void spr_vfree_pattern_set(uint8_t patrn_idx) {
  uint8_t npat, size;

  SpritePattern *ps = &spr_pattern[patrn_idx];

  size = ps->size;
  if (ps->size == SPR_SIZE_32x16)
    size = 8;
  if (ps->size == SPR_SIZE_32x32)
    size = 16;

  npat = ps->n_planes * ps->n_steps * size;
  ps->allocated = false;
  sys_memset(&spr_patt_valloc[ps->pidx], 1, npat);
}

bool spr_is_allocated(uint8_t patrn_idx) {
  SpritePattern *ps = &spr_pattern[patrn_idx];
  return ps->allocated;
}

static void spr_calc_patterns(SpriteDef *sp) __nonbanked {
  uint8_t i, color_frame, base = 0, base2, frame;

  SpritePattern *ps = sp->pattern_set;
  for (i = 0; i < sp->cur_state; i++) {
    base += ps->state_steps[i];
  }
  color_frame = (base + sp->cur_anim_step) * ps->n_planes;

  switch (ps->size) {
  case SPR_SIZE_16x16:
    base *= (ps->size * ps->n_planes);
    frame = sp->cur_anim_step * (ps->size * ps->n_planes);
    for (i = 0; i < ps->n_planes; i++) {
      (sp->planes[i]).color |= (ps->colors2)[color_frame + i];
      (sp->planes[i]).pattern = ps->pidx + base + frame + i * ps->size;
    }
    break;
  case SPR_SIZE_16x32:
    base *= (SPR_SIZE_16x16 * ps->n_planes);
    base2 = base + ps->n_planes * ps->n_steps * SPR_SIZE_16x16;
    frame = sp->cur_anim_step * (SPR_SIZE_16x16 * ps->n_planes);
    for (i = 0; i < ps->n_planes; i++) {
      (sp->planes[i]).color &= 128;
      (sp->planes[i]).color |= (ps->colors2)[color_frame + i];
      (sp->planes[i + 2]).color &= 128;
      (sp->planes[i + 2]).color |= (ps->colors2)[color_frame + i];
      (sp->planes[i]).pattern = ps->pidx + base + frame + i * SPR_SIZE_16x16;
      (sp->planes[i + 2]).pattern =
          ps->pidx + base2 + frame + i * SPR_SIZE_16x16;
    }
    break;
  case SPR_SIZE_32x16:
    base *= (SPR_SIZE_16x32 * ps->n_planes); // 0 8 16 32
    base2 = base + 4;
    frame = sp->cur_anim_step * SPR_SIZE_16x32 * ps->n_planes; // 0 or 8
    for (i = 0; i < ps->n_planes; i++) {
      // 2 is the max number of planes supported
      (sp->planes[i]).color |= (ps->colors2)[color_frame];
      (sp->planes[i + 2]).color |= (ps->colors2)[color_frame];
      (sp->planes[i]).pattern = ps->pidx + base + frame + i * SPR_SIZE_16x32;
      (sp->planes[i + 2]).pattern =
          ps->pidx + base2 + frame + i * SPR_SIZE_16x32;
    }
    break;
  case SPR_SIZE_32x32:
    // only 1 plane supported
    base *= SPR_SIZE_16x32;
    base2 = base + ps->n_steps * SPR_SIZE_16x32;
    frame = sp->cur_anim_step * SPR_SIZE_16x32;
    (sp->planes[0]).color |= (ps->colors2)[color_frame];
    (sp->planes[1]).color |= (ps->colors2)[color_frame];
    (sp->planes[2]).color |= (ps->colors2)[color_frame];
    (sp->planes[3]).color |= (ps->colors2)[color_frame];
    (sp->planes[0]).pattern = ps->pidx + base + frame;
    (sp->planes[1]).pattern = ps->pidx + base + frame + 4;
    (sp->planes[2]).pattern = ps->pidx + base2 + frame;
    (sp->planes[3]).pattern = ps->pidx + base2 + frame + 4;
    break;
  }
}

void spr_update(SpriteDef *sp) __nonbanked {
  uint8_t i;
  spr_calc_patterns(sp);
  for (i = 0; i < sp->pattern_set->n_planes; i++) {
    sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i], (uint8_t *)&sp->planes[i],
               4);
    if (sp->pattern_set->size == SPR_SIZE_16x32 ||
        sp->pattern_set->size == SPR_SIZE_32x16) {
      sys_memcpy((uint8_t *)&spr_attr[sp->aidx + i + 1],
                 (uint8_t *)&sp->planes[i + 2], 4);
    } else if (sp->pattern_set->size == SPR_SIZE_32x32) {
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
 * spr_show: finds a gap to allocate the attribute set
 */
uint8_t spr_show(SpriteDef *sp) __nonbanked {
  uint8_t i, idx = 7, n, f = 0;
  n = sp->pattern_set->n_planes;
  if (sp->pattern_set->size == SPR_SIZE_16x32 ||
      sp->pattern_set->size == SPR_SIZE_32x16)
    n = n * 2;
  else if (sp->pattern_set->size == SPR_SIZE_32x32)
    n = n * 4;
  for (i = 0; i < vdp_hw_max_sprites - 1; i++) {
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

void spr_hide(SpriteDef *sp) __nonbanked {
  uint8_t n, idx;
  struct vdp_hw_sprite null_spr;

  n = sp->pattern_set->n_planes;
  if (sp->pattern_set->size == SPR_SIZE_16x32 ||
      sp->pattern_set->size == SPR_SIZE_32x16)
    n = n * 2;
  else if (sp->pattern_set->size == SPR_SIZE_32x32)
    n = n * 4;
  idx = sp->aidx;
  sys_memset(&spr_attr_valloc[idx], 1, n);

  /** set sprite outside screen using EC bit */
  null_spr.y = 193;
  null_spr.x = 0;
  null_spr.pattern = 0;
  null_spr.color = 128; // EC bit

  // FIXME: still wrong handling of multiple planes
  sys_memcpy((uint8_t *)&spr_attr[sp->aidx], (uint8_t *)&null_spr,
             sizeof(struct vdp_hw_sprite));
  if (sp->pattern_set->size == SPR_SIZE_16x32 ||
      sp->pattern_set->size == SPR_SIZE_32x16) {
    sys_memcpy((uint8_t *)&spr_attr[sp->aidx + 1], (uint8_t *)&null_spr,
               sizeof(struct vdp_hw_sprite));
  } else if (sp->pattern_set->size == SPR_SIZE_32x32) {
    // TODO
  }
}

/**
 * Set sprite position taking into account off-screen coordinates
 */
void spr_set_pos(SpriteDef *sp, int16_t xp, int16_t yp) __nonbanked {
  uint8_t i, x, y, ec = 0;

  if (yp > -33 && yp < 0)
    y = (int8_t)yp;
  else if (yp == 0)
    y = 0xFF;
  else if (yp > 0 && yp < 193)
    y = yp - 1;
  // need to cover > 192 as well

  if (xp < 0) {
    x = xp + 32;
    ec = 128;
  } else if (xp >= 0 && xp < 256)
    x = xp;

  for (i = 0; i < sp->pattern_set->n_planes; i++) {
    (sp->planes[i]).x = x;
    (sp->planes[i]).y = y;
    (sp->planes[i]).color = ec;
    if (sp->pattern_set->size == SPR_SIZE_16x32) {
      (sp->planes[i + 2]).x = x;
      (sp->planes[i + 2]).y = y + 16;
      (sp->planes[i + 2]).color = ec;
    } else if (sp->pattern_set->size == SPR_SIZE_32x16) {
      (sp->planes[i + 2]).x = x + 16;
      (sp->planes[i + 2]).y = y;
      (sp->planes[i + 2]).color = ec;
    } else if (sp->pattern_set->size == SPR_SIZE_32x32) {
      (sp->planes[1]).x = x + 16;
      (sp->planes[1]).y = y;
      (sp->planes[1]).color = ec;
      (sp->planes[2]).x = x;
      (sp->planes[2]).y = y + 16;
      (sp->planes[2]).color = ec;
      (sp->planes[3]).x = x + 16;
      (sp->planes[3]).y = y + 16;
      (sp->planes[3]).color = ec;
    }
  }
}

void spr_set_plane_colors(SpriteDef *sp, uint8_t *colors) __nonbanked {
  uint8_t i;
  for (i = 0; i < sp->pattern_set->n_planes; i++) {
    (sp->planes[i]).color = colors[i];
    if (sp->pattern_set->size == SPR_SIZE_16x32 ||
        sp->pattern_set->size == SPR_SIZE_32x16) {
      (sp->planes[i + 2]).color = colors[i];
    } else if (sp->pattern_set->size == SPR_SIZE_32x32) {
      (sp->planes[1]).color = colors[i];
      (sp->planes[2]).color = colors[i];
      (sp->planes[3]).color = colors[i];
    }
  }
}

/**
 * Handle sprite animation for simple cases of 2 and 4 states with collision
 */
void spr_animate(SpriteDef *sp, signed char dx, signed char dy) __nonbanked {
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
    // if (collision) {
    /* animate faster when colliding */
    //	sp->anim_ctr++;
    //}
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
