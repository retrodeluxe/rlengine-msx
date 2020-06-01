#ifndef _SCENE_H_
#define _SCENE_H_

#define NEXT_OBJECT(X)	sizeof(struct map_object_item) - sizeof(union map_object) + sizeof(X)

#define SCENE_MAX_DPO	31
#define SCENE_MAX_BULLET 8

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
	TILE_CROSS_STATUS,
	TILE_HEART_STATUS,
	TILE_STAINED_GLASS,
	TILE_SPLASH,
	TILE_MAX,
};

enum map_tile_sets {
      MAP_TILESET_1,
      MAP_TILESET_2,
      MAP_TILESET_3,
      MAP_TILESET_4,
      MAP_TILESET_5,
      MAP_TILESET_MAX,
};

#define MAP_TILESET_4_POS     126
#define MAP_TILESET_5_POS     158

enum spr_patterns_t {
	PATRN_BAT,
	PATRN_RAT,
	PATRN_SPIDER,
	PATRN_TEMPLAR,
	PATRN_JEAN,
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
	PATRN_BULLET,
	PATRN_MAX,
};

enum sound_effects {
	SFX_JUMP,
	SFX_CHAIN,
	SFX_PICKUP_ITEM,
	SFX_DEATH,
	SFX_SHOOT,
	SFX_DOOR,
	SFX_SLASH,
};

enum rooms_t {
	ROOM_EVIL_CHAMBER,
	ROOM_PRAYER_OF_HOPE,
	ROOM_CHURCH_TOWER,
	ROOM_CHURCH_WINE_SUPPLIES,
	ROOM_BONFIRE,
	ROOM_FOREST,
	ROOM_GRAVEYARD,
	ROOM_CHURCH_ENTRANCE,
	ROOM_CHURCH_ALTAR,
	ROOM_HAGMAN_TREE,
	ROOM_CAVE_DRAGON,
	ROOM_CAVE_GHOST,
	ROOM_CATACOMBS_FLIES,
	ROOM_CATACOMBS,
	ROOM_HIDDEN_GARDEN,
	ROOM_CAVE_TUNNEL,
	ROOM_CAVE_LAKE,
	ROOM_CATACOMBS_WHEEL,
	ROOM_DEATH,
	ROOM_HIDDEN_RIVER,
	ROOM_CAVE_GATE,
	ROOM_EVIL_CHURCH,
	ROOM_EVIL_CHURCH_2,
	ROOM_EVIL_CHURCH_3,
	ROOM_SATAN,
};

extern struct list_head display_list;
extern struct spr_sprite_def jean_sprite;
extern struct displ_object dpo_jean;
extern struct list_head *elem;
extern struct displ_object *dpo;

extern uint8_t scr_tile_buffer[768];
extern uint8_t data_buffer[2100];
extern uint8_t stick;
extern uint8_t trigger;
extern unsigned char *map_map_segment_dict[25];
extern unsigned char *map_map_segment[25];
extern unsigned char *map_object_layer[25];

extern const unsigned char huntloop_song_pt3[];
extern const unsigned char church_song_pt3[];
extern const unsigned char prayerofhope_song_pt3[];
extern const unsigned int huntloop_song_pt3_len;
extern const unsigned int church_song_pt3_len;
extern const unsigned int prayerofhope_song_pt3_len;
extern const unsigned int hell_song_pt3[];
extern const unsigned int hell_song_pt3_len;
extern const unsigned int cave_song_pt3[];
extern const unsigned int cave_song_pt3_len;
extern const unsigned int evilfight_song_pt3[];
extern const unsigned int evilfight_song_pt3_len;
extern const char abbaye_sfx_afb[];
extern const unsigned int abbaye_sfx_afb_len;

extern void remove_tileobject(struct displ_object *dpo);
extern void update_tileobject(struct displ_object *dpo);

#endif
