
#ifndef _MSX_H_
#define _MSX_H_

#define DEBUG

// try compile with C99 and use bool
#define false 0
#define true  1

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef byte
typedef unsigned char byte;
#endif

#define asm__di                 __asm di __endasm
#define asm__ei                 __asm ei __endasm

struct gfx_tilebank {
	const byte *pattern;
	const byte *color;
};

struct gfx_tileset {
	byte start;
	byte end;
	struct gfx_tilebank *bank;
};

struct gfx_tilemap {
	uint cur_x;
	uint cur_y;
	byte w;
	byte h;
	const byte *map;
};

/* used to read from map header data */
struct gfx_tilemap_object {
	byte x;			/* map coordinates */
	byte y;
	byte tile;		/* top left tile */
};

struct gfx_sprite_def {
	byte x;			/* current position */
	byte y;
	byte tile;		/* current tile */
	byte cur_dir;
	byte cur_anim_step;
	byte n_dirs;
	byte n_anim_steps;
	byte patterns[4];
};

struct gfx_viewport {
	byte x;
	byte y;
	byte w;
	byte h;
};

struct gfx_map_pos {
	char x;
	char y;
};

#define gfx_screen_tile_w       32
#define gfx_screen_tile_h       24


extern const struct spr_delta_pos spr_stick2coords[];

extern void gfx_dyntile_show(struct gfx_tilemap_object *obj,
			     struct gfx_tilebank *tilebank, byte x, byte y,
			     byte * scrbuf);
extern void gfx_sprite_show(struct gfx_sprite_def *spr,
			    struct gfx_tilebank *tilebank, byte x, byte y,
			    byte * scrbuf);
extern void gfx_sprite_move(struct gfx_sprite_def *spr, byte dir, byte steps,
			    char collision);
extern void gfx_sprite_clear(struct gfx_sprite_def *spr);
extern void gfx_dyntile_clear(struct gfx_tilemap_object *obj);

extern void blk_inflate(byte * dict, byte * in, byte * out, uint data_size,
			byte width);

#endif				/* _MSX_H_ */
