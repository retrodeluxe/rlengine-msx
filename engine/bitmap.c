#define DEBUG

#include "log.h"
#include "bitmap.h"

inline static uint8_t get(uint8_t a, uint8_t pos) {
    return (a >> pos) & 1;
}

inline static void set(uint8_t *a, uint8_t pos) {
    *a |= 1 << pos;
}

inline static void reset(uint8_t *a, uint8_t pos) {
    *a &= ~(1 << pos);
}

uint8_t bitmap_get(uint8_t *bitmap, uint8_t pos) {
    return get(bitmap[pos/8], pos%8) != 0;
}

void bitmap_set(uint8_t *bitmap, uint8_t pos) {
    set(&bitmap[pos/8], pos%8);
}

void bitmap_reset(uint8_t *bitmap, uint8_t pos) {
    reset(&bitmap[pos/8], pos%8);
}

/**
 * bitmap_find_gap:
 *      find a sequence of n 1s in the bitmap
 */
uint8_t bitmap_find_gap(uint8_t *bitmap, uint8_t gap_size, uint8_t bitmap_size)
{
	uint8_t i, nfree = 0;
	for(i = 0, bitmap_size *= 8; i < bitmap_size; i++) {
		nfree = nfree * bitmap_get(bitmap,i) + bitmap_get(bitmap,i);
		if (nfree == gap_size)
			return i - gap_size + 1;
	}
	// TOOD: use errno
	return 255;
}
