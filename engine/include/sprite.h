#ifndef _MSX_H_SPRITE
#define _MSX_H_SPRITE

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


extern void spr_init(char spritesize, char zoom);
extern byte spr_valloc(struct spr_sprite_def *sp);
extern void spr_vfree(struct spr_sprite_def *sp);
extern void spr_set_pos(struct spr_sprite_def *sp, byte x, byte y);
extern void spr_set_plane_colors(struct spr_sprite_def *sp, byte * colors);
extern void spr_show(struct spr_sprite_def *sp);
extern void spr_hide(struct spr_sprite_def *sp);
extern void spr_move(struct spr_sprite_def *sp, byte dir, byte steps,
		     char collision);


#define DEFINE_HW_SPRITE(SPR_DEF, SIZE, PLANES, STEPS, DIRS, ANIM_TRH, PATTERNS, COLORS)	(SPR_DEF).size = (SIZE);\
 																				(SPR_DEF).n_planes = (PLANES);\
 																				(SPR_DEF).n_anim_steps = (STEPS);\
 																				(SPR_DEF).n_dirs = (DIRS); \
 																				(SPR_DEF).patterns = (PATTERNS); \
 																				(SPR_DEF).cur_anim_step = 0; \
 																				(SPR_DEF).cur_dir = 0;\
 																				(SPR_DEF).anim_ctr_treshold = (ANIM_TRH);\
 																				(SPR_DEF).anim_ctr = 0;\
 																				spr_set_plane_colors(&(SPR_DEF),(COLORS))

#define SPR_SIZE_8x8	1
#define SPR_SIZE_16x16	4
#define SPR_DIR_LR 		2
#define SPR_DIR_LRUP	4

#endif