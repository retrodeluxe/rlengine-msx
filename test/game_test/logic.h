#ifndef _LOGIC_H_
#define _LOGIC_H_


struct game_state_t {

	/* current room */
	uint8_t room;

	/* jean position when swithing rooms */
	uint8_t jean_x;
	uint8_t jean_y;

	/* cross and live count for state panel */
	uint8_t cross_cnt;
	uint8_t live_cnt;

	/* action item status */
	uint8_t hearth[9];
	uint8_t scroll[7];
	uint8_t cross[12];
	bool bell;
	uint8_t invisible_trigger[5];
	uint8_t checkpoint[8];
	uint8_t toggle[3];
	bool cross_switch;
	bool cross_switch_enable;
	bool door_trigger;
	uint8_t templar_ct;
	uint8_t templar_delay;
	bool death;
};

extern struct game_state_t game_state;

void pickup_heart(struct displ_object *dpo, uint8_t data);
void pickup_scroll(struct displ_object *dpo, uint8_t data);
void pickup_cross(struct displ_object *dpo, uint8_t data);
void checkpoint_handler(struct displ_object *dpo, uint8_t data);
void toggle_handler(struct displ_object *dpo, uint8_t data);
void bell_handler(struct displ_object *dpo, uint8_t data);
void crosswitch_handler(struct displ_object *dpo, uint8_t data);
// void set_trigger_handler_object(struct map_object_item *map_object);
void trigger_handler(struct displ_object *dpo, uint8_t data);
#endif
