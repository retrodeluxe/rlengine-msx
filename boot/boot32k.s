		.area _BOOT

		.db "A"
		.db "B"
		.dw bootstrap
		.dw 0,0,0,0,0,0
		
		; At this point the BIOS has detected the cartridge AB signature and
		; jumped here; we have the rom bios in bank 0, ram in bank 3, and rom
		; in bank 1 (this code).

		; The following code sets bank 2 to the same slot as bank 1 and continues
		; execution.
bootstrap:
		ld sp,#0xfc4a
       	ld a,#0xC9
      	ld (nopret),A
nopret:	nop
		call #0x0138
		rrca
		rrca
		call getslot
		ld	h,#0x80
		call 0x0024
		jp done
getslot:
		and	#0x03
		ld	c,a
		ld	b,#0
		ld	hl,#0xFCC1
		add	hl,bc
		ld	a,(HL)
		and	#0x80
		jr	z,exit
		or	c
		ld	c,a
		inc	hl
		inc	hl
		inc	hl
		inc	hl
		ld	a,(hl)
		and	#0x0C
exit:
		or	c				
		ret
done:
