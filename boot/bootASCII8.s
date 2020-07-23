		.globl __sdcc_banked_call
		.globl cur_page
		.globl _ascii8_set_data
		.globl _ascii8_set_code
		.globl _ascii8_restore

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

		; Mapper page switching helpers
		;
_ascii8_set_code:
		push 	ix
		ld	ix,#0
		add 	ix,sp
		ld	a,(cur_page)
		ld	(cur_page2),a
		ld	a,4(ix)
		ld 	(ASCII8_PAGE2),a
		ld	(cur_page),a
		pop 	ix
		ret
_ascii8_set_data:
		push 	ix
		ld	ix,#0
		add 	ix,sp
		ld	a,4(ix)
		ld 	(ASCII8_PAGE3),a
		pop 	ix
		ret
_ascii8_restore:
		push 	ix
		ld	a,(cur_page2)
		ld	(ASCII8_PAGE2),a
		pop	ix
		ret

		; workaround for LD A,I failing to set P/V if an interrupt occurs during the
		; instruction. We detect this by examining if (sp - 1) was overwritten.
		; f: pe <- interrupts enabled
		; Modifies: af
check_ei:
		xor	a
		push	af  ; set (sp - 1) to 0
		pop	af
		ld	a,i
		ret	pe  ; interrupts enabled? return with pe
		dec	sp
		dec	sp  ; check whether the Z80 lied about ints being disabled
		pop	af  ; (sp - 1) is overwritten w/ MSB of ret address if an ISR occurred
		sub	#1
		sbc	a,a
		and	#1  ; (sp - 1) is not 0? return with pe, otherwise po
		ret
		;
		; Support for SDCC baked calls
		;
__sdcc_banked_call:
		; save caller and current bank
		ld	a,i
		di
		pop 	bc
		ld	a,(cur_page)
		ld	iy,#0
		add	iy,sp
		ld 	sp,(banked_sp)
		push	bc
		push	af
		ld 	(banked_sp), sp
		ld 	sp,iy

		ld	iy,#0
		add	iy,bc

		; push return adrress
		ld	hl, #__sdcc_banked_ret
		push 	hl

		; read jump address and target page
		ld	l,(iy)
		ld 	h,1(iy)
		ld	a,2(iy)
		ld 	(cur_page),a

		; switch bank and perform call
		ld 	(ASCII8_PAGE2),a
		jp	po, $1
		ei
$1:
		jp	(hl)
__sdcc_banked_ret:
		ld	a,i
		di
		ld	iy,#0
		add	iy,sp
		ld 	sp, (banked_sp)
		pop 	af
		pop 	hl
		ld 	(banked_sp), sp
		ld 	sp,iy

		; restore bank
		ld 	(ASCII8_PAGE2),a
		ld 	(cur_page),a
		inc 	hl
		inc	hl
		inc	hl
		push	hl
		ret	po
		ei
		ret

		; The following code sets bank 2 to the same slot as bank 1 and continues
		; execution.
bootstrap:
		ld	a,#2
		ld	(cur_page),a
		ld	hl,#0xf000
		ld	(banked_sp),hl
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

		; initialize banked call stack



		.area _DATA
romslot:	.ds 1
biosslot:	.ds 1
cur_page:	.ds 1
cur_page2:	.ds 1
banked_sp:	.ds 2
