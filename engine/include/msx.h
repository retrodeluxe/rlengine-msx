
#ifndef _MSX_H_
#define _MSX_H_

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
#define irq_tics_cycle          60
#define irq_secs_cycle          60
#define irq_rate_1_hz           irq_tics_cycle
#define irq_rate_2_hz           irq_tics_cycle/2
#define irq_rate_5_hz           irq_tics_cycle/5
#define irq_rate_10_hz          irq_tics_cycle/10
#define irq_rate_60_hz          1
#define irq_rate_1_sec          1
#define irq_rate_6_sec          irq_secs_cycle/10
#define irq_rate_10_sec         irq_secs_cycle/6

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

struct ay_reg_map {
	byte A_tone_fine;
	byte A_tone_coarse;
	byte B_tone_fine;
	byte B_tone_coarse;
	byte C_tone_fine;
	byte C_tone_coarse;
	byte Noise_period;
	byte Mixer_Ctl;
	byte A_Amp_Ctl;
	byte B_Amp_Ctl;
	byte C_Amp_Ctl;
	byte Env_period_fine;
	byte Env_period_coarse;
	byte Env_shape;
};

extern const struct spr_delta_pos spr_stick2coords[];

extern void sys_reboot();
extern void sys_init_rom_32k();
extern byte sys_get_key(byte line);
extern byte sys_get_stick(byte port);
extern void sys_memcpy(byte * dst, byte * src, uint size);
#define         sys_memset __builtin_memset
#define         sys_init_stack(X)   __asm     \
                                    di        \
                                    ld sp,(X) \
                                    ei        \
                                    __endasm

extern void irq_init(void);
extern void irq_start(void);
extern void irq_register(void (*func), byte tic_rate, byte sec_rate);

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
extern void gfx_tileset_to_vram(struct gfx_tileset *ts, byte bank);
extern void gfx_tilemap_clip(struct gfx_tilemap *tm, struct gfx_viewport *vp,
			     byte * scrbuf, struct gfx_map_pos *p);
#define         gfx_set_tile_vram(_x,_y,_tile) vdp_poke(vdp_base_names_grp1+32*_y+_x, _tile)

extern void psg_set_all(struct ay_reg_map *regs);
extern void psg_set_tone(unsigned int period, byte chan);
extern void psg_set_noise(byte period);
extern void psg_set_mixer(byte mixval);
extern void psg_set_vol(byte chan, byte vol);
extern void psg_set_envelope(unsigned int period, byte shape);

extern void blk_inflate(byte * dict, byte * in, byte * out, uint data_size,
			byte width);

#endif				/* _MSX_H_ */
