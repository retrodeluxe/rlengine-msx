		.globl _sys_set_bios
		.globl _sys_set_rom
    .globl _sys_memcpy_rom

		.area _BOOT

RSLREG		.equ 0x0138
EXPTBL		.equ 0xfcc1
SLOT_SEL	.equ 0xa8
VDP_DW		.equ 0x0007
VDP_DW_RAM	.equ 0xf379
SUBSLOT_SEL	.equ 0xffff

		.db "A"
		.db "B"
		.dw bootstrap
		.dw 0,0,0,0,0,0

		; At this point the BIOS has detected the cartridge AB signature and
		; jumped here; we have the rom bios in bank 0, ram in bank 3, and rom
		; in bank 1 (this code).

		; Utility functions to switch slot in page 0 and 3

_sys_set_rom:
		push ix
		di
		ld	a,(romslot)
		ld	h,#0x00
		call	enaslt
    ld  a,(romslot)
		pop ix
		ret
_sys_set_bios:
		push ix
		ld	a,(biosslot)
		ld	h, #0x00
		call	enaslt
		ei
		pop ix
		ret

    ; copy a buffer from rom in page 3 to ram in page 3
_sys_memcpy_rom:
    ret

		; set slot and subslot at target address
		; (from msx bios listing)
		; hl - target address
		; a  - slot : FxxxSSPP
		;             F  : expanded slot flag (if F=0, SS is ignored)
		;             SS : expanded subslot
		;             PP : primary slot
enaslt:
		call selprm         ; calculate bit pattern and mask code
		jp m, eneslt        ; if expanded set secondary first
		in a,(SLOT_SEL)
		and c
		or b
		out (SLOT_SEL),a        ; set primary slot
		ret
eneslt:
		push hl
		call selexp         ; set secondary slot
		pop hl
		jr enaslt

		; calculate bit pattern and mask
selprm:
		di
		push af
		ld a,h
		rlca
		rlca
		and #3
		ld e,a              ; bank number
		ld a,#0xC0
selprm1:
		rlca
		rlca
		dec e
		jp p, selprm1
		ld e,a              ; mask pattern
		cpl
		ld c,a              ; inverted mask pattern
		pop af
		push af
		and #3              ; extract xxxxxxPP
		inc a
		ld b,a
		ld a,#0xAB
selprm2:
		add a,#0x55
		djnz selprm2
		ld d,a              ; primary slot bit pattern
		and e
		ld b,a
		pop af
		and a               ; if expanded slot set sign flag
		ret

		; set secondary slot
selexp:
		push af
		ld a,d
		and #0xC0          ; get slot number for bank 3
		ld c,a
		pop af
		push af
		ld d,a
		in a,(SLOT_SEL)
		ld b,a
		and #0x3F
		or c
		out (SLOT_SEL),a        ; set bank 3 to target slot
		ld a,d
		rrca
		rrca
		and #3
		ld d,a
		ld a,#0xAB          ; secondary slot to bit pattern
selexp1:
		add a,#0x55
		dec d
		jp p,selexp1
		and e
		ld d,a
		ld a,e
		cpl
		ld h,a
		ld a,(SUBSLOT_SEL)       ; read and update secondary slot register
		cpl
		ld l,a
		and h               ; strip off old bits
		or d                ; add new bits
		ld (SUBSLOT_SEL),a
		ld a,b
		out (SLOT_SEL),a        ; restore status
		pop af
		and #3
		ret

		; The following code sets bank 2 to the same slot as bank 1 and continues
		; execution.
bootstrap:
		ld 	sp,#0xf378
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
		call 	enaslt

		; backup VDP_RW to RAM
		ld  a,(#VDP_DW)
		ld  (#VDP_DW_RAM),a
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

		.area _DATA
romslot:	.ds 1
biosslot:	.ds 1
