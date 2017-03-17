#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "sprite.h"
#include "wq.h"
#include "tile.h"
#include "map.h"
#include "log.h"
#include "displ.h"
#include "phys.h"
#include "list.h"

#include "logic.h"

struct game_state_t game_state;

void init_game_state()
{
	// room 3
	game_state.map_x = 96;
	game_state.map_y = 44;
}


void pickup_heart()
{
	log_e("picking up a hearth!!\n");
}
