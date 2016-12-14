
#ifndef _MSX_H_
#define _MSX_H_

#define DEBUG

// try compile with C99 and use bool
#define false 0
#define true  1

#define asm__di                 __asm di __endasm
#define asm__ei                 __asm ei __endasm

struct gfx_tilebank {
	const uint8_t *pattern;
	const uint8_t *color;
};

struct gfx_tileset {
	uint8_t start;
	uint8_t end;
	struct gfx_tilebank *bank;
};

struct gfx_tilemap {
	uint16_t cur_x;
	uint16_t cur_y;
	uint8_t w;
	uint8_t h;
	const uint8_t *map;
};

/* used to read from map header data */
struct gfx_tilemap_object {
	uint8_t x;			/* map coordinates */
	uint8_t y;
	uint8_t tile;		/* top left tile */
};

struct gfx_sprite_def {
	uint8_t x;			/* current position */
	uint8_t y;
	uint8_t tile;		/* current tile */
	uint8_t cur_dir;
	uint8_t cur_anim_step;
	uint8_t n_dirs;
	uint8_t n_anim_steps;
	uint8_t patterns[4];
};

struct gfx_viewport {
	uint8_t x;
	uint8_t y;
	uint8_t w;
	uint8_t h;
};

struct gfx_map_pos {
	char x;
	char y;
};

#define gfx_screen_tile_w       32
#define gfx_screen_tile_h       24


extern const struct spr_delta_pos spr_stick2coords[];

extern void gfx_dyntile_show(struct gfx_tilemap_object *obj,
			     struct gfx_tilebank *tilebank, uint8_t x, uint8_t y,
			     uint8_t * scrbuf);
extern void gfx_sprite_show(struct gfx_sprite_def *spr,
			    struct gfx_tilebank *tilebank, uint8_t x, uint8_t y,
			    uint8_t * scrbuf);
extern void gfx_sprite_move(struct gfx_sprite_def *spr, uint8_t dir, uint8_t steps,
			    char collision);
extern void gfx_sprite_clear(struct gfx_sprite_def *spr);
extern void gfx_dyntile_clear(struct gfx_tilemap_object *obj);

extern void blk_inflate(uint8_t * dict, uint8_t * in, uint8_t * out, uint16_t data_size,
			uint8_t width);

#endif				/* _MSX_H_ */
