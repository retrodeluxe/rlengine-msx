;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.1.0 #7066 (Nov 22 2011) (Mac OS X i386)
; This file was generated Sat Sep 10 17:31:52 2016
;--------------------------------------------------------
	.module hello_msx
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _vdp_print_grp1
	.globl _vdp_clear_grp1
	.globl _vdp_set_color
	.globl _vdp_set_mode
	.globl _sys_get_key
	.globl _sys_reboot
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
;  ram data
;--------------------------------------------------------
	.area _DATA
;--------------------------------------------------------
; overlayable items in  ram 
;--------------------------------------------------------
	.area _OVERLAY
;--------------------------------------------------------
; external initialized ram data
;--------------------------------------------------------
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:9: void main()
;	---------------------------------
; Function main
; ---------------------------------
_main_start::
_main:
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:11: vdp_set_mode(vdp_grp1);
	ld	a,#0x01
	push	af
	inc	sp
	call	_vdp_set_mode
	inc	sp
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:12: vdp_set_color(vdp_white, vdp_black);
	ld	hl,#0x010F
	push	hl
	call	_vdp_set_color
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:13: vdp_clear_grp1(0);
	ld	h,#0x00
	ex	(sp),hl
	inc	sp
	call	_vdp_clear_grp1
	inc	sp
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:15: vdp_print_grp1(10, 10, "Hello MSX");
	ld	hl,#__str_0
	push	hl
	ld	hl,#0x0A0A
	push	hl
	call	_vdp_print_grp1
	pop	af
	pop	af
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:17: do {
00101$:
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:18: } while (sys_get_key(8) & 1);
	ld	a,#0x08
	push	af
	inc	sp
	call	_sys_get_key
	inc	sp
	ld	a,l
	and	a, #0x01
	jr	NZ,00101$
;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:20: sys_reboot();
	jp	_sys_reboot
_main_end::
__str_0:
	.ascii "Hello MSX"
	.db 0x00
	.area _CODE
	.area _CABS
