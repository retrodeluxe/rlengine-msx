#ifndef _TILE_H_
#define _TILE_H_

#define gfx_screen_tile_w       32
#define gfx_screen_tile_h       24

struct tile_set {
	byte w;
	byte h;
	const byte *pattern;
	const byte *color;
};

struct tile_map {
	uint cur_x;
	uint cur_y;
	byte w;
	byte h;
	const byte *map;
};

#define INIT_TILE_SET(SET, GEN)		(SET).w = GEN ## _tile_w;\
									(SET).h = GEN ## _tile_h;\
									(SET).pattern = GEN ## _tile;\
									(SET).color = GEN ## _tile_color

extern void tile_set_to_vram_bank(struct tile_set *ts, byte bank, byte offset);
extern void tile_set_to_vram(struct tile_set *ts, byte pos);
extern void tile_map_clip(struct tile_map *tm, struct gfx_viewport *vp,
			     byte * scrbuf, struct gfx_map_pos *p);
#define     set_tile_vram(_x,_y,_tile) vdp_poke(vdp_base_names_grp1+32*_y+_x, _tile)

#endif