#ifndef _LOGIC_H_
#define _LOGIC_H_


struct game_state_t {
	uint8_t map_x;	// position on the map in tile coordinates
	uint8_t map_y;
	uint8_t cross_cnt;
	uint8_t live_cnt;
};

extern struct game_state_t game_state;

void pickup_heart();

#endif
