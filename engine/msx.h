
#ifndef _MSX_H_
#define _MSX_H_

/* -- debug -- */
#define DEBUG

#define LOG_ERROR   0
#define LOG_DEBUG   1
#define LOG_WARNING 2
#define LOG_INFO    3
#define LOG_VERBOSE 4
#define LOG_ENTRY   5
#define LOG_EXIT  	6

#define LOGLEVEL 7
/* -- debug -- */

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

struct vdp_hw_sprite {
	byte y;
	byte x;
	byte pattern;
	byte color;
};

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

struct spr_sprite_def {
	struct vdp_hw_sprite planes[3];
	byte cur_dir;
	byte cur_anim_step;
	char auto_inc_x;
	char auto_inc_y;
	byte anim_ctr;
	byte anim_ctr_treshold;
	byte base_hw_attr;
	byte base_hw_patt;
	byte size;		/* 1:8x8 4:16x16 */
	byte type;
	byte n_planes;
	byte n_dirs;
	byte n_anim_steps;
	const byte *patterns;
};

struct spr_delta_pos {
	char dx;
	char dy;
};

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


#define vdp_txt 0
#define vdp_grp1 1
#define vdp_grp2 2
#define vdp_mult 3

#define vdp_trasnp	0
#define vdp_black 	1
#define vdp_green	2
#define vdp_lgreen	3
#define vdp_dblue	4
#define vdp_blue 	5
#define vdp_dred	6
#define vdp_lblue	7
#define vdp_red 	8
#define vdp_lred	9
#define vdp_yellow	10
#define vdp_lyellow	11
#define vdp_dgreen	12
#define vdp_magenta	13
#define vdp_grey	14
#define vdp_white	15

#define vdp_base_names_grp1     0x1800
#define vdp_base_color_grp1     0x2000
#define vdp_base_chars_grp1     0x0000
#define vdp_base_spatr_grp1     0x1b00
#define vdp_base_sppat_grp1     0x3800

#define vdp_hw_max_sprites      32
#define vdp_hw_max_patterns     255
#define gfx_screen_tile_w       32
#define gfx_screen_tile_h       24

extern void vdp_screen_disable(void);
extern void vdp_screen_enable(void);
extern void vdp_set_mode(char mode);
extern void vdp_set_color(char ink, char border);
extern void vdp_poke(uint address, byte value);
extern byte vdp_peek(uint address);
extern void vdp_memset(uint vaddress, uint size, byte value);
extern void vdp_copy_to_vram(byte * buffer, uint vaddress, uint length);
extern void vdp_copy_to_vram_di(byte * buffer, uint vaddress, uint length);
extern void vdp_copy_from_vram(uint vaddress, byte * buffer, uint length);
extern void vdp_set_hw_sprite(char spi, struct vdp_hw_sprite *spr);
extern void vdp_set_hw_sprite_di(byte * spr, byte spi);
extern void vdp_init_hw_sprites(char spritesize, char zoom);
extern void vdp_fastcopy_nametable(byte * buffer);
extern void vdp_fastcopy_nametable_di(byte * buffer);
extern void vdp_fastcopy16(byte * src_ram, uint dst_vram);
extern void vdp_clear_grp1(byte color);
extern void vdp_print_grp1(char x, char y, char *msg);

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

extern void spr_init(char spritesize, char zoom);
extern byte spr_valloc(struct spr_sprite_def *sp);
extern void spr_vfree(struct spr_sprite_def *sp);
extern void spr_set_pos(struct spr_sprite_def *sp, byte x, byte y);
extern void spr_set_plane_colors(struct spr_sprite_def *sp, byte * colors);
extern void spr_show(struct spr_sprite_def *sp);
extern void spr_hide(struct spr_sprite_def *sp);
extern void spr_move(struct spr_sprite_def *sp, byte dir, byte steps,
		     char collision);

extern void psg_set_all(struct ay_reg_map *regs);
extern void psg_set_tone(unsigned int period, byte chan);
extern void psg_set_noise(byte period);
extern void psg_set_mixer(byte mixval);
extern void psg_set_vol(byte chan, byte vol);
extern void psg_set_envelope(unsigned int period, byte shape);

extern void blk_inflate(byte * dict, byte * in, byte * out, uint data_size,
			byte width);

#ifdef DEBUG
extern void log(int level, char *fmt, ...);
extern void dump_vram(int start_addr, int end_addr);

#define log_d(_fmt, ...)  log(LOG_DEBUG, _fmt, ##__VA_ARGS__)
#define log_w(_fmt, ...)  log(LOG_WARNING ,_fmt, ##__VA_ARGS__)
#define log_i(_fmt, ...)  log(LOG_INFO, _fmt, ##__VA_ARGS__)
#define log_v(_fmt, ...)  log(LOG_VERBOSE, _fmt, ##__VA_ARGS__)
#define log_e(_fmt, ...)  log(LOG_ERROR, _fmt, ##__VA_ARGS__)
#define log_entry(_fmt, ...)  log(LOG_ENTRY, _fmt, ##__VA_ARGS__)
#define log_exit(_fmt, ...)  log(LOG_EXIT, _fmt, ##__VA_ARGS__)
#else
#define log_d(_fmt, ...) 
#define log_w(_fmt, ...)  
#define log_i(_fmt, ...)  
#define log_v(_fmt, ...)  
#define log_e(_fmt, ...)  
#define log_entry(_fmt, ...)  
#define log_exit(_fmt, ...) 

#endif				/* DEBUG */

#endif				/* _MSX_H_ */
