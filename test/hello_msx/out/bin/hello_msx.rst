                              1 ;--------------------------------------------------------
                              2 ; File Created by SDCC : free open source ANSI-C Compiler
                              3 ; Version 3.1.0 #7066 (Nov 22 2011) (Mac OS X i386)
                              4 ; This file was generated Sat Sep 10 17:31:52 2016
                              5 ;--------------------------------------------------------
                              6 	.module hello_msx
                              7 	.optsdcc -mz80
                              8 	
                              9 ;--------------------------------------------------------
                             10 ; Public variables in this module
                             11 ;--------------------------------------------------------
                             12 	.globl _main
                             13 	.globl _vdp_print_grp1
                             14 	.globl _vdp_clear_grp1
                             15 	.globl _vdp_set_color
                             16 	.globl _vdp_set_mode
                             17 	.globl _sys_get_key
                             18 	.globl _sys_reboot
                             19 ;--------------------------------------------------------
                             20 ; special function registers
                             21 ;--------------------------------------------------------
                             22 ;--------------------------------------------------------
                             23 ;  ram data
                             24 ;--------------------------------------------------------
                             25 	.area _DATA
                             26 ;--------------------------------------------------------
                             27 ; overlayable items in  ram 
                             28 ;--------------------------------------------------------
                             29 	.area _OVERLAY
                             30 ;--------------------------------------------------------
                             31 ; external initialized ram data
                             32 ;--------------------------------------------------------
                             33 ;--------------------------------------------------------
                             34 ; global & static initialisations
                             35 ;--------------------------------------------------------
                             36 	.area _HOME
                             37 	.area _GSINIT
                             38 	.area _GSFINAL
                             39 	.area _GSINIT
                             40 ;--------------------------------------------------------
                             41 ; Home
                             42 ;--------------------------------------------------------
                             43 	.area _HOME
                             44 	.area _HOME
                             45 ;--------------------------------------------------------
                             46 ; code
                             47 ;--------------------------------------------------------
                             48 	.area _CODE
                             49 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:9: void main()
                             50 ;	---------------------------------
                             51 ; Function main
                             52 ; ---------------------------------
   403A                      53 _main_start::
   403A                      54 _main:
                             55 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:11: vdp_set_mode(vdp_grp1);
   403A 3E 01                56 	ld	a,#0x01
   403C F5                   57 	push	af
   403D 33                   58 	inc	sp
   403E CD A0 55             59 	call	_vdp_set_mode
   4041 33                   60 	inc	sp
                             61 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:12: vdp_set_color(vdp_white, vdp_black);
   4042 21 0F 01             62 	ld	hl,#0x010F
   4045 E5                   63 	push	hl
   4046 CD C4 55             64 	call	_vdp_set_color
                             65 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:13: vdp_clear_grp1(0);
   4049 26 00                66 	ld	h,#0x00
   404B E3                   67 	ex	(sp),hl
   404C 33                   68 	inc	sp
   404D CD FD 57             69 	call	_vdp_clear_grp1
   4050 33                   70 	inc	sp
                             71 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:15: vdp_print_grp1(10, 10, "Hello MSX");
   4051 21 6E 40             72 	ld	hl,#__str_0
   4054 E5                   73 	push	hl
   4055 21 0A 0A             74 	ld	hl,#0x0A0A
   4058 E5                   75 	push	hl
   4059 CD 52 58             76 	call	_vdp_print_grp1
   405C F1                   77 	pop	af
   405D F1                   78 	pop	af
                             79 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:17: do {
   405E                      80 00101$:
                             81 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:18: } while (sys_get_key(8) & 1);
   405E 3E 08                82 	ld	a,#0x08
   4060 F5                   83 	push	af
   4061 33                   84 	inc	sp
   4062 CD 9C 4F             85 	call	_sys_get_key
   4065 33                   86 	inc	sp
   4066 7D                   87 	ld	a,l
   4067 E6 01                88 	and	a, #0x01
   4069 20 F3                89 	jr	NZ,00101$
                             90 ;/Users/geijoenr/Projects/MSX/rlengine-msx1/test/hello_msx/hello_msx.c:20: sys_reboot();
   406B C3 98 4F             91 	jp	_sys_reboot
   406E                      92 _main_end::
   406E                      93 __str_0:
   406E 48 65 6C 6C 6F 20    94 	.ascii "Hello MSX"
        4D 53 58
   4077 00                   95 	.db 0x00
                             96 	.area _CODE
                             97 	.area _CABS
