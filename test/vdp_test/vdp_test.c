/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

#include "msx.h"
#include "vdp.h"
#include "gen/vdp_test.h"

void main()
{
	
	vdp_set_mode(vdp_grp1);

	// I think I need some more highlevel stuff


//extern void vdp_screen_disable(void);
//extern void vdp_screen_enable(void);
//extern void vdp_set_mode(char mode);
//extern void vdp_set_color(char ink, char border);


//extern void vdp_poke(uint address, byte value);
//extern byte vdp_peek(uint address);
//extern void vdp_memset(uint vaddress, uint size, byte value);
//extern void vdp_copy_to_vram(byte * buffer, uint vaddress, uint length);
//extern void vdp_copy_to_vram_di(byte * buffer, uint vaddress, uint length);
//extern void vdp_copy_from_vram(uint vaddress, byte * buffer, uint length);

// THESE ARE GRP1 SPECIFIC
// unless I make them genericsc	
//extern void vdp_set_hw_sprite(char spi, struct vdp_hw_sprite *spr);
//extern void vdp_set_hw_sprite_di(byte * spr, byte spi);
//extern void vdp_init_hw_sprites(char spritesize, char zoom);
//extern void vdp_fastcopy_nametable(byte * buffer);
//extern void vdp_fastcopy_nametable_di(byte * buffer);
//extern void vdp_fastcopy16(byte * src_ram, uint dst_vram);
//extern void vdp_clear_grp1(byte color);
}