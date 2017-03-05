#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>

uint8_t bitmap_get  (uint8_t *bitmap, uint8_t index);
void bitmap_set   (uint8_t *bitmap, uint8_t index);
void bitmap_reset (uint8_t *bitmap, uint8_t index);
uint8_t bitmap_find_gap(uint8_t *bitmap, uint8_t gap_size, uint8_t bitmap_size);

#endif
