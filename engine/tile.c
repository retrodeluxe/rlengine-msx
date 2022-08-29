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

#include "tile.h"
#include "bitmap.h"
#include "log.h"
#include "msx.h"
#include "sys.h"
#include "vdp.h"

#pragma CODE_PAGE 2

#define BANK1_OFFSET 256 * 8
#define BANK2_OFFSET BANK1_OFFSET * 2

#define BITMAP_TILEBANK_SIZE 32 /* 32 * 8 tiles */

uint8_t bitmap_tile_bank[BITMAP_TILEBANK_SIZE];
uint8_t bitmap_tile_bank0[BITMAP_TILEBANK_SIZE];
uint8_t bitmap_tile_bank1[BITMAP_TILEBANK_SIZE];
uint8_t bitmap_tile_bank2[BITMAP_TILEBANK_SIZE];

/*
 * Dynamically allocated array of TileSets
 */
TileSet *tile_sets;

/**
 * This function clears all TileSet allocations.
 */
void tile_init() {
  /* initialize bitmap to all ones : free */
  sys_memset(bitmap_tile_bank, 255, BITMAP_TILEBANK_SIZE);
  sys_memset(bitmap_tile_bank0, 255, BITMAP_TILEBANK_SIZE);
  sys_memset(bitmap_tile_bank1, 255, BITMAP_TILEBANK_SIZE);
  sys_memset(bitmap_tile_bank2, 255, BITMAP_TILEBANK_SIZE);

  // first tile is reserved
  bitmap_reset(bitmap_tile_bank, 0);
  bitmap_reset(bitmap_tile_bank0, 0);
  bitmap_reset(bitmap_tile_bank1, 0);
  bitmap_reset(bitmap_tile_bank2, 0);
}

/**
 * Transfer a compressed Tileset to VRAM into a fixed position and into a specific bank.
 *
 * This function is useful when a TileMap requires tile defintions to be at a
 * specific offset.
 *
 *   .. warning::
 *
 *        this function overrides previous TileSet allocations.
 *
 * :param tileset: TileSet to be transferred
 * :param bank: target bank see :c:type:`TileBank`
 * :param offset: start position inside the target bank to transder the TileSet
 */
 void tile_set_to_vram_bank(TileSet *tileset, TileBank bank, uint8_t offset) {
  uint16_t size, _offset, i;
  _offset = offset * 8;
  size = tileset->w * tileset->h * 8;
  if (bank == BANK0 || bank == ALLBANKS) {
    vdp_rle_inflate(VRAM_BASE_PTRN + _offset, tileset->pattern, size);
    vdp_rle_inflate(VRAM_BASE_COLR + _offset, tileset->color, size);
  }
  if (bank == BANK1 || bank == ALLBANKS) {
    vdp_rle_inflate(VRAM_BASE_PTRN + _offset + BANK1_OFFSET, tileset->pattern, size);
    vdp_rle_inflate(VRAM_BASE_COLR + _offset + BANK1_OFFSET, tileset->color, size);
  }
  if (bank == BANK2 || bank == ALLBANKS) {
    vdp_rle_inflate(VRAM_BASE_PTRN + _offset + BANK2_OFFSET, tileset->pattern, size);
    vdp_rle_inflate(VRAM_BASE_COLR + _offset + BANK2_OFFSET, tileset->color, size);
  }
  for (i = offset; i < offset + (size / 8); i++) {
    bitmap_reset(bitmap_tile_bank, i);
    if (bank == BANK0 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank0, i);
    else if (bank == BANK1 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank1, i);
    else if (bank == BANK2 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank2, i);
  }

  tileset->allocated = true;
  tileset->pidx = offset;
}


/**
 * Transfer an uncompressed Tileset to VRAM into a fixed position and a specific bank.
 *
 * This function is useful when a TileMap requires tile defintions to be at a
 * specific offset.
 *
 *   .. warning::
 *
 *        this function overrides previous TileSet allocations.
 *
 * :param tileset: TileSet to be transferred
 * :param bank: target bank see :c:type:`TileBank`
 * :param offset: start position inside the target bank to transder the tileset
 */
void tile_set_to_vram_bank_raw(TileSet *tileset, TileBank bank, uint8_t offset) {
  uint16_t size, _offset, i;
  _offset = offset * 8;
  size = tileset->w * tileset->h * 8;
  if (bank == BANK0 || bank == ALLBANKS) {
    vdp_memcpy(VRAM_BASE_PTRN + _offset, tileset->pattern, size);
    vdp_memcpy(VRAM_BASE_COLR + _offset, tileset->color, size);
  }
  if (bank == BANK1 || bank == ALLBANKS) {
    vdp_memcpy(VRAM_BASE_PTRN + _offset + BANK1_OFFSET, tileset->pattern, size);
    vdp_memcpy(VRAM_BASE_COLR + _offset + BANK1_OFFSET, tileset->color, size);
  }
  if (bank == BANK2 || bank == ALLBANKS) {
    vdp_memcpy(VRAM_BASE_PTRN + _offset + BANK2_OFFSET, tileset->pattern, size);
    vdp_memcpy(VRAM_BASE_COLR + _offset + BANK2_OFFSET, tileset->color, size);
  }

  for (i = offset; i < offset + (size / 8); i++) {
    bitmap_reset(bitmap_tile_bank, i);
    if (bank == BANK0 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank0, i);
    else if (bank == BANK1 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank1, i);
    else if (bank == BANK2 || bank == ALLBANKS)
      bitmap_reset(bitmap_tile_bank2, i);
  }

  tileset->allocated = true;
  tileset->pidx = offset;
}

/**
 * Allocates and transfers a TileSet to VRAM in a specific tile bank
 *
 * This function transders uncompressed TileSets to the first available
 * gap in VRAM in the specified bank.
 *
 * :param tileset: the TileSet to be allocated and transferred
 *
 * :returns: either EOK or error code
 */
int tile_set_valloc_bank(TileSet *tileset, TileBank bank) {
  uint8_t pos = 0, size;
  bool f = false;

  if (tileset->allocated) {
    return EALREADY;
  }

  size = tileset->w * tileset->h;

  if (bank == BANK0)
   f = bitmap_find_gap(bitmap_tile_bank0, size, BITMAP_TILEBANK_SIZE - 1, &pos);
  else if (bank == BANK1)
   f = bitmap_find_gap(bitmap_tile_bank1, size, BITMAP_TILEBANK_SIZE - 1, &pos);
  else if (bank == BANK2)
   f = bitmap_find_gap(bitmap_tile_bank2, size, BITMAP_TILEBANK_SIZE - 1, &pos);

  if (!f)
    return ENOMEM;

  tile_set_to_vram_bank(tileset, bank, pos);
}

/**
 * Allocates and transfers a TileSet to VRAM.
 *
 * This function detects compression and transfers the TileSet to the first available
 * gap in VRAM.
 *
 * :param tileset: the TileSet to be allocated and transferred
 *
 * :returns: either EOK or error code
 */
int tile_set_valloc(TileSet *tileset) {
  uint16_t offset, vsize;
  uint8_t i, pos, size;
  bool found;

  if (tileset->allocated) {
    return EALREADY;
  }

  size = tileset->w * tileset->h;

  found =
      bitmap_find_gap(bitmap_tile_bank, size, BITMAP_TILEBANK_SIZE - 1, &pos);
  if (!found) {
    return ENOMEM;
  }

  for (i = pos; i < pos + size; i++) {
    bitmap_reset(bitmap_tile_bank, i);
    bitmap_reset(bitmap_tile_bank0, i);
    bitmap_reset(bitmap_tile_bank1, i);
    bitmap_reset(bitmap_tile_bank2, i);
  }

  offset = pos * 8;
  vsize = size * 8;
  if (tileset->raw) {
    vdp_memcpy(VRAM_BASE_PTRN + offset, tileset->pattern, vsize);
    vdp_memcpy(VRAM_BASE_COLR + offset, tileset->color, vsize);
    vdp_memcpy(VRAM_BASE_PTRN + offset + BANK1_OFFSET, tileset->pattern, vsize);
    vdp_memcpy(VRAM_BASE_COLR + offset + BANK1_OFFSET, tileset->color, vsize);
    vdp_memcpy(VRAM_BASE_PTRN + offset + BANK2_OFFSET, tileset->pattern, vsize);
    vdp_memcpy(VRAM_BASE_COLR + offset + BANK2_OFFSET, tileset->color, vsize);
  } else {
    vdp_rle_inflate(VRAM_BASE_PTRN + offset, tileset->pattern, vsize);
    vdp_rle_inflate(VRAM_BASE_COLR + offset, tileset->color, vsize);
    vdp_rle_inflate(VRAM_BASE_PTRN + offset + BANK1_OFFSET, tileset->pattern, vsize);
    vdp_rle_inflate(VRAM_BASE_COLR + offset + BANK1_OFFSET, tileset->color, vsize);
    vdp_rle_inflate(VRAM_BASE_PTRN + offset + BANK2_OFFSET, tileset->pattern, vsize);
    vdp_rle_inflate(VRAM_BASE_COLR + offset + BANK2_OFFSET, tileset->color, vsize);
  }
  tileset->allocated = true;
  tileset->pidx = pos;
  return EOK;
}

/**
 * Transfer a Tileset to VRAM into a fixed position in `ALL_BANKS`
 *
 * This function is useful when a TileMap requires tile defintions to be at a
 * specific offset.
 *
 *   .. warning::
 *
 *        this function overrides previous TileSet allocations.
 *
 * :param tileset: TileSet to be transferred
 * :param offset: start position inside the target bank to transder the tileset
 */
void tile_set_to_vram(TileSet *tileset, uint8_t offset) {
  if (tileset->allocated)
    return;

  if (tileset->raw)
    tile_set_to_vram_bank_raw(tileset, ALLBANKS, offset);
  else
    tile_set_to_vram_bank(tileset, ALLBANKS, offset);
}

// FIXME: remove
void tile_set_to_vram_raw(TileSet *tileset, uint8_t offset) {
  if (tileset->allocated)
    return;

  tile_set_to_vram_bank_raw(tileset, ALLBANKS, offset);
}

/**
 * Free a previously allocated TileSet
 *
 * :param tileset: the TileSet to be freed.
 */
void tile_set_vfree(TileSet *tileset) {
  uint8_t i, size;

  if (!tileset->allocated)
    return;
  size = tileset->w * tileset->h;
  for (i = tileset->pidx; i < tileset->pidx + size; i++)
    bitmap_set(bitmap_tile_bank, i);
  tileset->allocated = false;
  tileset->pidx = 0;
}


/*
 * vram transfer using tileset indexes
 */
int tile_valloc(uint8_t tileset_idx)
{
  return tile_set_valloc(&tile_sets[tileset_idx]);
}

int tile_valloc_bank(uint8_t tileset_idx, TileBank bank)
{
  return tile_set_valloc_bank(&tile_sets[tileset_idx], bank);
}

void tile_to_vram_bank(uint8_t tileset_idx, TileBank bank, uint8_t offset)
{
  // TODO: check for raw flag
  tile_set_to_vram_bank(&tile_sets[tileset_idx], bank, offset);
}

void tile_to_vram(uint8_t tileset_idx, uint8_t offset)
{
  // TODO: check for raw flag
  tile_set_to_vram(&tile_sets[tileset_idx], offset);
}

void tile_vfree(uint8_t tileset_idx)
{
  tile_set_vfree(&tile_sets[tileset_idx]);
}

/**
 * Transfers a TileObject into a Buffer and/or VRAM
 *
 * :param tileobject: the TileObject to be displayed
 * :param scrbuf: a screen buffer in RAM
 * :param refresh_vram: true for the TileObject to be transferred to VRAM after
 *    transferring into the RAM buffer.
 *
 */
void tile_object_show(TileObject *tileobject, uint8_t *scrbuf,
                      bool refresh_vram) __nonbanked {
  uint16_t offset = tileobject->x / 8 + tileobject->y / 8 * 32;
  uint8_t *ptr = scrbuf + offset;
  uint8_t tile_base = tileobject->tileset->pidx;
  uint8_t tile = tileobject->tileset->pidx + tileobject->idx;
  uint8_t x, y;

  tile += (tileobject->tileset->frame_w * tileobject->frame) +
          tileobject->state * (tileobject->tileset->frame_w * tileobject->tileset->frames);

  for (y = 0; y < tileobject->tileset->frame_h; y++) {
    for (x = 0; x < tileobject->tileset->frame_w; x++) {
      *(ptr + x) = tile;
      if (refresh_vram) {
        vdp_write(VRAM_BASE_NAME + offset + x, tile);
      }
      tile++;
      /** allow for shift using idx: this will only work
                      for single frame one direction objects **/
      if (tileobject->idx > 0 && tile > tile_base + tileobject->tileset->frame_w - 1)
        tile = tile_base;
    }
    ptr += 32;
    offset += 32;
    tile += tileobject->tileset->w - tileobject->tileset->frame_w;
  }
}

/**
 * Hide a TileObject by replacing it with zero-tiles
 *
 * :param to: the TileObject to be hidden
 * :param scrbuf: a screen buffer in RAM
 * :param refresh_vram: true for the TileObject to be transferred to VRAM after
 *    transferring into the RAM buffer.
 */
void tile_object_hide(TileObject *tileobject, uint8_t *scrbuf,
                      bool refresh_vram) __nonbanked {
  uint16_t offset = tileobject->x / 8 + tileobject->y / 8 * 32;
  uint8_t *ptr = scrbuf + offset;
  uint8_t x, y;

  for (y = 0; y < tileobject->tileset->frame_h; y++) {
    for (x = 0; x < tileobject->tileset->frame_w; x++) {
      *(ptr + x) = 0;
      if (refresh_vram) {
        vdp_write(VRAM_BASE_NAME + offset + x, 0);
      }
    }
    ptr += 32;
    offset += 32;
  }
}
