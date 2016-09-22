#ifndef _MSX_H_SPRITE
#define _MSX_H_SPRITE


#define spr_size_small	1
#define spr_size_big	4
#define spr_dir_lr 		1
#define spr_dir_lrud 	2
#define spr_dir_lrudc 	3
#define spr_dir_all 	4


/**
 * spr_sprite_pattern_set:
 *		a set of sprite patterns plus data used for animation
 */
struct spr_sprite_pattern_set {
	byte pidx;
	byte size;
	byte n_planes;
	byte n_dirs;
	byte n_anim_steps;
	const byte *patterns;
};

/**
 * spr_sprite_def:
 *		copy of sprite attributes as in vram plus animation status
 */
struct spr_sprite_def {
	byte aidx;
	struct vdp_hw_sprite planes[3];
	struct spr_sprite_pattern_set *pattern_set;
	byte cur_dir;
	byte cur_anim_step;
	char auto_inc_x;
	char auto_inc_y;
	byte anim_ctr;
	byte anim_ctr_treshold;
};


struct spr_delta_pos {
	char dx;
	char dy;
};

/**
 * helper macros for sprite definition from generated data
 */
#define SPR_DEFINE_PATTERN_SET(X, SIZE, PLANES, DIRS, STEPS, PATTERNS) 		(X).size = (SIZE);\
 																			(X).n_planes = (PLANES);\
 																			(X).n_anim_steps = (STEPS);\
 																			(X).n_dirs = (DIRS); \
 																			(X).patterns = (PATTERNS)

#define SPR_DEFINE_SPRITE(X, PATTERN_SET, ANIM_TRH, COLORS)					(X).pattern_set = (PATTERN_SET);\
 																			(X).cur_anim_step = 0; \
 																			(X).cur_dir = 0;\
 																			(X).anim_ctr_treshold = (ANIM_TRH);\
 																			(X).anim_ctr = 0;\
 																			spr_set_plane_colors(&(X),(COLORS))

extern void spr_init(char spritesize, char zoom);
extern byte spr_valloc_pattern_set(struct spr_sprite_pattern_set *ps);
extern void spr_vfree_pattern_set(struct spr_sprite_pattern_set *ps);
extern void spr_set_pos(struct spr_sprite_def *sp, byte x, byte y);
extern void spr_set_plane_colors(struct spr_sprite_def *sp, byte * colors);
extern byte spr_show(struct spr_sprite_def *sp);
extern void spr_update(struct spr_sprite_def *sp);
extern void spr_hide(struct spr_sprite_def *sp);
extern void spr_animate(struct spr_sprite_def *sp, signed char dx, signed char dy,
		     char collision);

#endif