		.area _BOOT

ENASLT		.equ 0x0024
RSLREG		.equ 0x0138
EXPTBL		.equ 0xfcc1

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
		ld 	sp,#0xf379
		ld	a,#0xc9
		ld 	(nopret),a
nopret:		nop
		call 	#RSLREG
		rrca
		rrca
		call 	getslot
		ld	h,#0x80
		call 	#ENASLT
		jp 	done
getslot:
		and	#0x03
		ld	c,a
		ld	b,#0
		ld	hl,#EXPTBL
		add	hl,bc
		ld	a,(hl)
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
