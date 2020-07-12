#ifndef _SCENE_H_
#define _SCENE_H_

#define NEXT_OBJECT(X)	sizeof(struct map_object_item) - sizeof(union map_object) + sizeof(X)

#define SCENE_MAX_DPO	31
#define SCENE_MAX_BULLET 9
#define SCENE_MAX_TOB_BULLET 8

enum tile_sets_t {
	TILE_SCROLL,
	TILE_RED_SCROLL,
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
	TILE_HARMLESS_SPEAR,
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
	TILE_BLOCK_CROSS,
	TILE_INVERTED_CROSS,
	TILE_FLAME,
	TILE_ENDING,
	TILE_EXPLOSION,
	TILE_PENTAGRAM,
	TILE_MAX,
};

enum map_tile_sets {
      MAP_TILESET_FOREST,
      MAP_TILESET_CHURCH_1,
      MAP_TILESET_CHURCH_2,
      MAP_TILESET_TREES_1,
      MAP_TILESET_CAVE_1,
      MAP_TILESET_CATACOMBS_1,
      MAP_TILESET_BEAM,
      MAP_TILESET_STONE,
      MAP_TILESET_FLAMES,
      MAP_TILESET_BRICKS,
      MAP_TILESET_BRICKS_2,
      MAP_TILESET_PLANT,
      MAP_TILESET_FOREST_2,
      MAP_TILESET_GRAVES,
      MAP_TILESET_CHURCH_DECO,
      MAP_TILESET_CHURCH_DECO_2,
      MAP_TILESET_X,
      MAP_TILESET_HANGING_PLANT,
      MAP_TILESET_SKULL,
      MAP_TILESET_DEATH_DECO,
      MAP_TILESET_SKULL_2,
      MAP_TILESET_ROPE,
      MAP_TILESET_CAVE_2,
      MAP_TILESET_DEBUG,
      MAP_TILESET_MAX,
};

/** positions of individual map tilesets **/
enum map_tileset_idx {
      MAP_TILESET_FOREST_POS = 1,
      MAP_TILESET_CHURCH_1_POS = 6,
      MAP_TILESET_CHURCH_2_POS = 16,
      MAP_TILESET_TREES_1_POS = 28,
      MAP_TILESET_CAVE_1_POS = 32,
      MAP_TILESET_CATACOMBS_1_POS = 39,
      MAP_TILESET_BEAM_POS = 44,
      MAP_TILESET_STONE_POS = 46,
      MAP_TILESET_FLAMES_POS = 57,
      MAP_TILESET_BRICKS_POS = 61,
      MAP_TILESET_BRICKS_2_POS = 65,
      MAP_TILESET_PLANT_POS = 126,
      MAP_TILESET_FOREST_2_POS = 127,
      MAP_TILESET_GRAVES_POS = 138,
      MAP_TILESET_CHURCH_DECO_POS = 150,
      MAP_TILESET_CHURCH_DECO_2_POS = 158,
      MAP_TILESET_X_POS = 161,
      MAP_TILESET_HANGING_PLANT_POS = 163,
      MAP_TILESET_SKULL_POS = 164,
      MAP_TILESET_DEATH_DECO_POS = 165,
      MAP_TILESET_SKULL_2_POS = 174,
      MAP_TILESET_ROPE_POS = 180,
      MAP_TILESET_CAVE_2_POS = 181,
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
	PATRN_SCYTHE,
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
	PATRN_ARROW,
	PATRN_SPIT,
	PATRN_SMALL_BULLET,
	PATRN_HEARTH_MASK,
	PATRN_CROSS_MASK,
	PATRN_MAX,
};

enum sound_effects {
	SFX_JUMP,
	SFX_SWITCH,
	SFX_PICKUP_ITEM,
	SFX_DEATH,
	SFX_SHOOT,
	SFX_DOOR,
	SFX_PORTAL
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
	ROOM_MAX,
};

extern struct list_head display_list;
extern struct spr_sprite_def jean_sprite;
extern struct displ_object dpo_jean;
extern struct list_head *elem;
extern struct displ_object *dpo;

extern uint8_t scr_tile_buffer[768];
extern uint8_t data_buffer[2100];
extern uint8_t sfx_buffer[431];
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
void init_scene();

#endif
