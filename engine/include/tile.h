#ifndef _TILE_H_
#define _TILE_H_

#define gfx_screen_tile_w       32
#define gfx_screen_tile_h       24

struct tile_set {
	uint8_t w;
	uint8_t h;
	const uint8_t *pattern;
	const uint8_t *color;
	uint8_t pidx;
	bool allocated;
};

struct tile_map {
	uint16_t cur_x;
	uint16_t cur_y;
	uint8_t w;
	uint8_t h;
	const uint8_t *map;
};

struct tile_object {
	uint8_t x;
	uint8_t y;
	uint8_t size;  /*< 0:1x1, 1:2x2, 2:4x4 */
	struct tile_set *ts;
	uint8_t idx; /*< index of first tile in the tile set */
};

#define INIT_TILE_SET(SET, GEN)	(SET).w = GEN ## _tile_w;\
				(SET).h = GEN ## _tile_h;\
				(SET).pattern = GEN ## _tile;\
				(SET).color = GEN ## _tile_color; \
				(SET).allocated = false;

extern void tile_init();
extern void tile_set_valloc(struct tile_set *ts);
extern void tile_set_to_vram_bank(struct tile_set *ts, uint8_t bank, uint8_t offset);
extern void tile_set_to_vram(struct tile_set *ts, uint8_t pos);
extern void tile_map_clip(struct tile_map *tm, struct gfx_viewport *vp,
			     uint8_t * scrbuf, struct gfx_map_pos *p);


extern void tile_object_show(struct tile_object *to, uint8_t * scrbuf);
#define     set_tile_vram(_x,_y,_tile) vdp_poke(vdp_base_names_grp1+32*_y+_x, _tile)

#endif
