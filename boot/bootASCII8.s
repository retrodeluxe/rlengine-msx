		.globl _sys_set_ascii_page3

		.area _BOOT

ENASLT		.equ 0x0024
RSLREG		.equ 0x0138
EXPTBL		.equ 0xfcc1
ASCII8_PAGE1	.equ 0x6800
ASCII8_PAGE2    .equ 0x7000
ASCII8_PAGE3	.equ 0x7800

		.db "A"
		.db "B"
		.dw bootstrap
		.dw 0,0,0,0,0,0

		; At this point the BIOS has detected the cartridge AB signature and
		; jumped here; we have the rom bios in bank 0, ram in bank 3, and rom
		; in bank 1 (this code).

		; utility function to swap ASCII8 rom page in 0xA000-0xBFFF
_sys_set_ascii_page3:
		push 	ix
		ld	ix,#0
		add	ix, sp
		ld      a,4(ix)
		ld 	(ASCII8_PAGE3),a
		pop     ix
		ret

		; The following code sets bank 2 to the same slot as bank 1 and continues
		; execution.
bootstrap:
		ld 	sp,#0xf379
		ld	a,#0xc9
		ld 	(nopret),a
nopret:		nop
		call 	#RSLREG
		call 	getslot
		ld	(biosslot),a
		call 	#RSLREG
		rrca
		rrca
		call 	getslot
		ld	(romslot),a
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
		; Now make sure ASCII8 pages are set properly
		ld 	a,#1
		ld 	(ASCII8_PAGE1),a
		ld 	a,#2
		ld 	(ASCII8_PAGE2),a
		ld 	a,#3
		ld 	(ASCII8_PAGE3),a

		.area _DATA
romslot:	.ds 1
biosslot:	.ds 1
