#ifndef _SCENE_H_
#define _SCENE_H_

#define NEXT_OBJECT(X)	sizeof(struct map_object_item) - sizeof(union map_object) + sizeof(X)

// NOTE: whenever you add something here remember to  initialize the pattern,
//       because free_patterns() assumes that is the case
enum tile_sets_t {
	TILE_SCROLL,
	TILE_CHECKPOINT,
	TILE_CROSS,
	TILE_HEART,
	TILE_BELL,
	TILE_SWITCH,
	TILE_TOGGLE,
	TILE_TELETRANSPORT,
	TILE_CUP,
	TILE_DRAGON,
	TILE_LAVA,
	TILE_SPEAR,
	TILE_WATER,
	TILE_SATAN,
	TILE_ARCHER_SKELETON,
	TILE_GARGOLYNE,
	TILE_PLANT,
	TILE_PRIEST,
	TILE_DOOR,
	TILE_TRAPDOOR,
	TILE_INVISIBLE_TRIGGER,
	TILE_FONT_DIGITS,
	TILE_FONT_UPPER,
	TILE_FONT_LOWER,
	TILE_MAX,
};

enum spr_patterns_t {
	PATRN_BAT,
	PATRN_RAT,
	PATRN_SPIDER,
	PATRN_TEMPLAR,
	PATRN_MONK,
	PATRN_WORM,
	PATRN_SKELETON,
	PATRN_PALADIN,
	PATRN_GUADANYA,
	PATRN_GHOST,
	PATRN_DEMON,
	PATRN_DEATH,
	PATRN_DARKBAT,
	PATRN_FLY,
	PATRN_SKELETON_CEILING,
	PATRN_FISH,
	PATRN_FIREBALL,
	PATRN_WATERDROP,
	PATRN_MAX,
};

enum rooms_t {
	ROOM_EVIL_CHAMBER,
	ROOM_MOON_SIGHT,
	ROOM_CHURCH_TOWER,
	ROOM_CHURCH_UPPER_FLOOR,
	ROOM_BONFIRE,
	ROOM_FOREST,
	ROOM_GRAVEYARD,
	ROOM_CHURCH_ENTRANCE,
	ROOM_CHURCH,
};

extern struct list_head display_list;
extern struct spr_sprite_def monk_sprite;
extern struct displ_object dpo_monk;

extern uint8_t scr_tile_buffer[768];
extern uint8_t stick;

extern void remove_tileobject(struct displ_object *dpo);
extern void update_tileobject(struct displ_object *dpo);

#endif
