/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "gen/bitmap_test.h"
#include <stdlib.h>

void main()
{
	vdp_set_mode(MODE_GRP2);
	vdp_set_color(COLOR_WHITE, COLOR_BLACK);
	vdp_clear(0);

  for(;;);
}
