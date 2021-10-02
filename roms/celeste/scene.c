/**
 *
 * Copyright (C) Retro DeLuxe 2021, All rights reserved.
 *
 */
#include "dpo.h"
#include "font.h"
#include "list.h"
#include "log.h"
#include "map.h"
#include "msx.h"
#include "phys.h"
#include "pt3.h"
#include "sfx.h"
#include "sprite.h"
#include "sys.h"
#include "tile.h"
#include "vdp.h"
#include "ascii8.h"

#include <stdlib.h>

#pragma CODE_PAGE 4

// with this and the graphics we should be able to build static scenes
// for the whole map.

//-- object types --
//------------------
enum {
	type_player_spawn = 1,
	type_player,
	type_platform,
	type_key = 8,
	type_spring = 18,
	type_chest = 20,
	type_balloon = 22,
	type_fall_floor = 23,
	type_fruit = 26,
	type_fly_fruit = 28,
	type_fake_wall = 64,
	type_message = 86,
	type_big_chest = 96,
	type_flag = 118,
	type_smoke,
	type_orb,
	type_lifeup
};
