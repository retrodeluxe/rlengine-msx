/*
 *  SDCC Vortex Tracker II PT3 player for MSX
 *
 * - Vortex Tracker II v1.0 PT3 player for ZX Spectrum by S.V.Bulba
 *	 <vorobey@mail.khstu.ru> http://bulba.at.kz
 *  - (09-Jan-05) Adapted to MSX by Alfonso D. C. aka Dioniso <dioniso072@yahoo.es>
 *  - Arrangements for MSX ROM: MSXKun/Paxanga soft >
 *    http://paxangasoft.retroinvaders.com/
 *  - asMSX version: SapphiRe > http://www.z80st.es/
 *  - Adapted to SDCC: mvac7/303bcn > <mvac7303b@gmail.com>
 *  - Clean up and refactoring by Retro Deluxe 2019
 *
 * Limitations of this version:
 * - No version detection (just for Vortex Tracker II and PT3.5).
 * - No frequency table decompression (default is number 2).
 * - No volume table decompression (Vortex Tracker II/PT3.5 volume table used).
 */

#include "pt3.h"
#include "sys.h"
#include "msx.h"

#pragma CODE_PAGE 2

uint8_t chana[29]; //CHNPRM_Size
uint8_t chanb[29];
uint8_t chanc[29];

uint8_t delay_cnt;
uint16_t CurESld;
uint8_t CurEDel;

uint8_t Ns_Base;
uint8_t AddToNs;

uint8_t AYREGS[14];
uint16_t EnvBase;
uint8_t VAR0END[240];

/*
 * set bit0 to 1 to disable song loop, bit7 indicates playback looped over
 */
uint8_t pt3_setup;

/* pointer to song data */
uint16_t pt3_song;

/* cursor position in pattern */
uint16_t PT3_CrPsPtr;

/* sample platterns */
uint16_t PT3_SAMPTRS;

/* ornament */
uint16_t PT3_OrnPtrs;

uint16_t PT3_PDSP;     //pilasave
uint16_t PT3_CSP;      //pilsave2
uint16_t PT3_PSP;      //pilsave3

uint8_t PT3_PrNote;
uint16_t PT3_PrSlide;

uint16_t PT3_AdInPtA;  //play data pattern
uint16_t PT3_AdInPtB;  //play data
uint16_t PT3_AdInPtC;  //play data

uint16_t PT3_LPosPtr;  //Position Ptr?
uint16_t PT3_PatsPtr;  //Pat Ptr

uint8_t PT3_Delay;            //delay
uint8_t PT3_AddToEn;          //Envelope data (No cal ya que no usa Envs??)
uint8_t PT3_Env_Del;          //Envelope data (idem)
uint16_t PT3_ESldAdd;  //Envelope data (idem)


uint8_t pt3_note_table[192];       //Note table

/**
 * Silence the PSG
 */
void pt3_mute() __naked
{
  __asm
mute:
  xor  a
  ld   (#_AYREGS+AR_AmplA),a
  ld   (#_AYREGS+AR_AmplB),a
  ld   (#_AYREGS+AR_AmplC),a
  jp   _pt3_play
  __endasm;
}

/**
 * Initialize the PT3 Module
 *
 * :param song: a PT3 song in binary format
 * :param loop: indicates whether the song should loop (0:no, 1: yes)
 */
void pt3_init(uint8_t *song,uint8_t loop) __naked
{
	unused(song);
	unused(loop);

	__asm
	push ix
	ld   ix,#0
	add  ix,sp

	ld   hl,#_pt3_setup
	ld   a,6(ix)
	or   a
	jr   nz, with_loop
	set  0,(hl)
	jr   init_song
with_loop:
	res  0,(hl)

init_song:
	ld   l,4(ix)
	ld   h,5(ix)
	call init_player
	pop  ix
	ret

init_player:
	ld   (#_pt3_song),hl
	push hl

	ld   de,#100
	add  hl,de

	ld   a,(hl)            ;+100 = 1B Delay
	ld   (#_PT3_Delay),a

	push hl
	pop  ix                 ;<<-- IX = _pt3_song + 100

  add  hl,de
	ld   (#_PT3_CrPsPtr),hl  ;+200 = Cr Ps Ptr data

	ld   e,2(ix)
	add  hl,de
	inc  hl
	ld   (#_PT3_LPosPtr),hl  ;

  pop  DE                 ;<<-- DE = _pt3_song

	ld   l,3(ix)
	ld   h,4(ix)
  add  hl,de
	ld   (#_PT3_PatsPtr),hl

	ld   hl,#169
	add  hl,de
	ld   (#_PT3_OrnPtrs),hl

	ld   hl,#105
	add  hl,de
	ld   (#_PT3_SAMPTRS),hl

	ld   hl,#_pt3_setup
	res  7,(hl)


	; Create Volume Table for Vortex Tracker II/PT3.5
	; (c) Ivan Roshin, adapted by SapphiRe ---
	ld	hl,#0x11
	ld	d,h
	ld  e,h
	ld  ix,#_VAR0END  ;_VT_+16
	ld  b,#15
INITV1:
	push hl
	add  hl,de
	ex   de,hl
	sbc  hl,hl
	ld   c,b
	ld   b,#16
INITV2:
	ld   a,l
	rla
	ld   a,h
	adc  a,#0
	ld   (ix),a
	inc  ix
	add  hl,de
	djnz INITV2
	pop  hl
	ld   a,e
	cp   #0x77
	jr   nz,INITV3
	inc  e
INITV3:
	ld   b,c
	djnz INITV1

; --- INITIALIZE PT3 VARIABLES ---
	xor	 a
	ld   hl,#_chana     ;VARS
	ld   (hl),a
	ld   de,#_chana+1   ;VARS+1
	ld   bc,#_VAR0END - _chana -1  ;VARS -1
	ldir

	inc  a
	ld   (#_delay_cnt),a
	ld   hl,#0xF001	;H - CHNPRM_Volume, L - CHNPRM_NtSkCn
	ld   (#_chana+CHNPRM_NtSkCn),hl
	ld   (#_chanb+CHNPRM_NtSkCn),hl
	ld   (#_chanc+CHNPRM_NtSkCn),hl

	ld   hl,#EMPTYSAMORN
	ld   (#_PT3_AdInPtA),hl ;ptr to zero  ; # chg
	ld   (#_chana+CHNPRM_OrnPtr),hl ;ornament 0 is "0,1,0"
	ld   (#_chanb+CHNPRM_OrnPtr),hl ;in all versions from
	ld   (#_chanc+CHNPRM_OrnPtr),hl ;3.xx to 3.6x and VTII

	ld   (#_chana+CHNPRM_SamPtr),hl ;S1 There is no default
	ld   (#_chanb+CHNPRM_SamPtr),hl ;S2 sample in PT3, so, you
	ld   (#_chanc+CHNPRM_SamPtr),hl ;S3 can comment S1,2,3; see
				    ;also EMPTYSAMORN comment
	ret

	__endasm;
}

/**
 * Initialize PT3 note table
 *  there are 4 available note tables in files pt3_nt0.h, pt3_nt1.h
 *  pt3_nt2.h and pt3_nt3.h. One of those files should be included to load the
 *  definition on a note table in the global variable NT. After that this
 *  function can be called passing NT as parameter.
 *
 * :param note_table: a PT3 note table in binary format.
 */
void pt3_init_notes(uint8_t *note_table)
{
	sys_memcpy(pt3_note_table, note_table, 96*2);
}

/**
 * Play a current set of notes.
 *   This function should be called after pt3_decode
 *
 */
void pt3_play() __naked __nonbanked
{
	__asm

	ld   a,(#_AYREGS+AR_Mixer)
	and  #0b00111111
	ld   b,a

	ld   a,#AR_Mixer
	out  (#AY0index),a
	in   a,(#AY0read)
	and	 #0b11000000	; Mascara para coger dos bits de joys
	or	 b		        ; A�ado Byte de B

	ld   (#_AYREGS+AR_Mixer),a

	xor  a

	ld   c,#AY0index
	ld   hl,#_AYREGS
LOUT:
	out (c),a
	inc  c
	outi
	dec  c
	inc  a
	cp   #13
	jr   nz,LOUT
	ret

__endasm;
}

/**
 * Decode the following note of the song
 *  this function should be called before pt3_play
 */
void pt3_decode() __naked __nonbanked
{
	__asm

	xor  A
	ld   (#_PT3_AddToEn),a
	ld   (#_AYREGS+AR_Mixer),a
	dec  A
	ld   (#_AYREGS+AR_EnvTp),a
	ld   hl,#_delay_cnt
	dec  (hl)
	JP   nz,PL2
	ld   hl,#_chana+CHNPRM_NtSkCn
	dec  (hl)
	jr   nz,PL1B
	ld	 bc,(#_PT3_AdInPtA)
	ld   a,(bc)
	and  A
	jr   nz,PL1A
	ld   d,a
	ld   (#_Ns_Base),a
	ld   hl,(#_PT3_CrPsPtr)
	inc  HL
	ld   a,(hl)
	inc  A
	jr   nz,PLNLP
	call CHECKLP
	ld	 hl,(#_PT3_LPosPtr)
	ld   a,(hl)
	inc  A
PLNLP:
	ld   (#_PT3_CrPsPtr),hl
	dec  A
	add  a,a
	ld   e,a
	RL   D
	ld   hl,(#_PT3_PatsPtr)
	add  hl,de
	ld   de,(#_pt3_song)
	ld   (#_PT3_PSP),sp
	ld   sp,hl
	pop  hl
	add  hl,de
	ld   b,h
	ld   c,l
	pop  hl
	add  hl,de
	ld   (#_PT3_AdInPtB),hl
	pop  hl
	add  hl,de
	ld   (#_PT3_AdInPtC),hl
	ld   sp,(#_PT3_PSP)

PL1A:
	ld   ix,#_chana+12
	call PTDECOD
	ld   (#_PT3_AdInPtA),BC

PL1B:
	ld   hl,#_chanb+CHNPRM_NtSkCn
	dec  (hl)
	jr   nz,PL1C
	ld   ix,#_chanb+12
	ld   bc,(#_PT3_AdInPtB)
	call PTDECOD
	ld   (#_PT3_AdInPtB),BC

PL1C:
	ld   hl,#_chanc+CHNPRM_NtSkCn
	dec  (hl)
	jr   nz,PL1D
	ld   ix,#_chanc+12
	ld   bc,(#_PT3_AdInPtC)
	call PTDECOD
	ld   (#_PT3_AdInPtC),BC

PL1D:
	ld   a,(#_PT3_Delay)
	ld   (#_delay_cnt),a

PL2:
	ld   ix,#_chana
	ld   hl,(#_AYREGS+AR_TonA)
	call CHREGS
	ld   (#_AYREGS+AR_TonA),hl
	ld   a,(#_AYREGS+AR_AmplC)
	ld   (#_AYREGS+AR_AmplA),a
	ld   ix,#_chanb
	ld   hl,(#_AYREGS+AR_TonB)
	call CHREGS
	ld   (#_AYREGS+AR_TonB),hl
	ld   a,(#_AYREGS+AR_AmplC)
	ld   (#_AYREGS+AR_AmplB),a
	ld   ix,#_chanc
	ld   hl,(#_AYREGS+AR_TonC)
	call CHREGS
	ld   (#_AYREGS+AR_TonC),hl

	ld   hl,(#_Ns_Base)    ;Ns_Base_AddToNs
	ld   a,h
	add  a,l
	ld   (#_AYREGS+AR_Noise),a

	ld   a,(#_PT3_AddToEn)
	ld   e,a
	add  a,a
	sbc  a,a
	ld   d,a
	ld   hl,(#_EnvBase)
	add  hl,de
	ld   de,(#_CurESld)
	add  hl,de
	ld  (#_AYREGS+AR_Env),hl

	xor  A
	ld   hl,#_CurEDel
	OR   (hl)

	ret Z
	dec  (hl)
	ret NZ
	ld   a,(#_PT3_Env_Del)
	ld   (hl),a
	ld   hl,(#_PT3_ESldAdd)
	add  hl,de
	ld   (#_CurESld),hl
	ret

CHECKLP:
	ld   hl,#_pt3_setup
	set  7,(hl)   ; -------------------------------------------------------------- <<< ????
	bit  0,(hl)   ;loop bit
	ret z

;=1 - No loop
	pop  hl
	ld   hl,#_delay_cnt
	inc  (hl)
	ld   hl,#_chana+CHNPRM_NtSkCn
	inc  (hl)
	jp   mute

PD_OrSm:
	ld   -12+CHNPRM_Env_En(ix),#0
	call SETORN
	ld   a,(bc)
	inc  bc
	rrca

PD_SAM:
	add  a,a
PD_SAM_:
	ld   e,a
	ld   d,#0
	ld	 hl,(#_PT3_SAMPTRS)
	add  hl,de
	ld   e,(hl)
	inc  hl
	ld   d,(hl)
	ld	 hl,(#_pt3_song)
	add  hl,de
	ld   -12+CHNPRM_SamPtr(ix),l
	ld   -12+CHNPRM_SamPtr+1(ix),h
	jr   PD_LOOP

PD_VOL:
	rlca
	rlca
	rlca
	rlca
	ld   -12+CHNPRM_Volume(ix),a
	jr   PD_LP2

PD_EOff:
	ld   -12+CHNPRM_Env_En(ix),a
	ld   -12+CHNPRM_PsInOr(ix),a
	jr   PD_LP2

PD_SorE:
	dec  A
	jr   nz,PD_ENV
	ld   a,(bc)
	inc  bc
	ld   -12+CHNPRM_NNtSkp(ix),a
	jr   PD_LP2

PD_ENV:
	call SETENV
	jr   PD_LP2

PD_ORN:
	call SETORN
	jr   PD_LOOP

PD_ESAM:
	ld   -12+CHNPRM_Env_En(ix),a
	ld   -12+CHNPRM_PsInOr(ix),a
	call nz,SETENV
	ld   a,(bc)
	inc  bc
	jr   PD_SAM_

PTDECOD:
	ld   a,-12+CHNPRM_Note(ix)
	ld   (#_PT3_PrNote),a           ;ld   (#PrNote+1),a
	ld   l,CHNPRM_CrTnSl-12(ix)
	ld   h,CHNPRM_CrTnSl+1-12(ix)
	ld  (#_PT3_PrSlide),hl

PD_LOOP:
	ld   de,#0x2010
PD_LP2:
	ld   a,(bc)
	inc  bc
	add  a,e
	jr   c,PD_OrSm
	add  a,d
	jr   z,PD_FIN
	jr   c,PD_SAM
	add  a,e
	jr   z,PD_REL
	jr   c,PD_VOL
	add  a,e
	jr   z,PD_EOff
	jr   c,PD_SorE
	add  a,#96
	jr   c,PD_NOTE
	add  a,e
	jr   c,PD_ORN
	add  a,d
	jr   c,PD_NOIS
	add  a,e
	jr   c,PD_ESAM
	add  a,a
	ld   e,a

  ld   hl,#(SPCCOMS + 0xDF20)  ;ld hl,((SPCCOMS+$DF20) % 65536)
;	push de
;	ld   de,#0xDF20
;	ld   hl,#SPCCOMS
;	add  hl,de
;	pop  de

	add  hl,de
	ld   e,(hl)
	inc  hl
	ld   d,(hl)
	push de

	jr   PD_LOOP

PD_NOIS:
  ld   (#_Ns_Base),a
	jr   PD_LP2

PD_REL:
  res  0,-12+CHNPRM_Flags(ix)
	jr   PD_RES

PD_NOTE:
  ld   -12+CHNPRM_Note(ix),a
	set  0,-12+CHNPRM_Flags(ix)
	xor  a

PD_RES:
	ld	 (#_PT3_PDSP),sp
	ld   sp,ix
	ld   h,a
	ld   l,a
	push hl
	push hl
	push hl
	push hl
	push hl
	push hl
	ld	sp,(#_PT3_PDSP)

PD_FIN:
  ld   a,-12+CHNPRM_NNtSkp(ix)
	ld   -12+CHNPRM_NtSkCn(ix),a
	ret

C_PORTM:
  res  2,-12+CHNPRM_Flags(ix)
	ld   a,(bc)
	inc  bc
;SKIP PRECALCULATED TONE DELTA (BECAUSE
;CANNOT BE RIGHT AFTER PT3 COMPILATION)
	inc  bc
	inc  bc
	ld   -12+CHNPRM_TnSlDl(ix),a
	ld   -12+CHNPRM_TSlCnt(ix),a
	ld   de,#_pt3_note_table
	ld   a,-12+CHNPRM_Note(ix)
	ld   -12+CHNPRM_SlToNt(ix),a
	add  a,a
	ld   l,a
	ld   h,#0
	add  hl,de
	ld   a,(hl)
	inc  HL
	ld   h,(hl)
	ld   l,a
	push HL
  ld   a,(#_PT3_PrNote)            ;<--- ld   a,#0x3E
	ld   -12+CHNPRM_Note(ix),a
	add  a,a
	ld   l,a
	ld   h,#0
	add  hl,de
	ld   e,(hl)
	inc  hl
	ld   d,(hl)
	pop  hl
	sbc  hl,de
	ld   -12+CHNPRM_TnDelt(ix),l
	ld   -12+CHNPRM_TnDelt+1(ix),h
  ld   de,(#_PT3_PrSlide)             ;<--- change to Kun version
	ld   -12+CHNPRM_CrTnSl(ix),E       ;<---
	ld   -12+CHNPRM_CrTnSl+1(ix),D     ;<---
  ld   a,(bc) ;SIGNED TONE STEP
	inc  bc
	ex   af,af
	ld   a,(bc)
	inc  bc
	and  a
	jr   z,NOSIG
	ex   de,hl
NOSIG:
  sbc  hl,de
	jp   P,SET_STP
	cpl
	ex   af,af
	neg
	ex   af,af
SET_STP:
  ld   -12+CHNPRM_TSlStp+1(ix),a
	ex   af,af
	ld   -12+CHNPRM_TSlStp(ix),a
	ld   -12+CHNPRM_COnOff(ix),#0
	ret

C_GLISS:
  set  2,-12+CHNPRM_Flags(ix)
	ld   a,(bc)
	inc  bc
	ld  -12+CHNPRM_TnSlDl(ix),a
	ld  -12+CHNPRM_TSlCnt(ix),a
	ld   a,(bc)
	inc  bc
	ex   af,af
	ld   a,(bc)
	inc  bc
	jr   SET_STP

C_SMPOS:
  ld   a,(bc)
	inc  bc
	ld   -12+CHNPRM_PsInSm(ix),a
	ret

C_ORPOS:
  ld   a,(bc)
	inc  bc
	ld   -12+CHNPRM_PsInOr(ix),a
	ret

C_VIBRT:
  ld   a,(bc)
	inc  bc
	ld   -12+CHNPRM_OnOffD(ix),a
	ld   -12+CHNPRM_COnOff(ix),a
	ld   a,(bc)
	inc  bc
	ld   -12+CHNPRM_OffOnD(ix),a
	xor  a
	ld   -12+CHNPRM_TSlCnt(ix),a
	ld   -12+CHNPRM_CrTnSl(ix),a
	ld   -12+CHNPRM_CrTnSl+1(ix),a
	ret

C_ENGLS:
  ld   a,(bc)
	inc  bc
	ld   (#_PT3_Env_Del),a
	ld   (#_CurEDel),a
	ld   a,(bc)
	inc  bc
	ld   l,a
	ld   a,(bc)
	inc  bc
	ld   h,a
	ld   (#_PT3_ESldAdd),hl
	ret

C_DELAY:
  ld   a,(bc)
	inc  bc
	ld   (#_PT3_Delay),a
	ret

SETENV:
  ld   -12+CHNPRM_Env_En(ix),E
	ld   (#_AYREGS+AR_EnvTp),a
	ld   a,(bc)
	inc  bc
	ld   h,a
	ld   a,(bc)
	inc  bc
	ld   l,a
	ld   (#_EnvBase),hl
	xor  A
	ld   -12+CHNPRM_PsInOr(ix),a
	ld   (#_CurEDel),a
	ld   h,a
	ld   l,a
	ld   (#_CurESld),hl

C_NOP:
  ret

SETORN:
  add  a,a
	ld   e,a
	ld   d,#0
	ld   -12+CHNPRM_PsInOr(ix),D
	ld	 hl,(#_PT3_OrnPtrs)
	add  hl,de
	ld   e,(hl)
	inc  hl
	ld   d,(hl)
	ld   hl,(#_pt3_song)
	add  hl,de
	ld   -12+CHNPRM_OrnPtr(ix),l
	ld   -12+CHNPRM_OrnPtr+1(ix),h
	ret





;-------------------------------------------------------------------------------
;ALL 16 ADDRESSES TO PROTECT FROM BROKEN PT3 MODULES

SPCCOMS:
.dw C_NOP			  ; ## COMPROBAR Q NO SEA AUTOMODIF
.dw C_GLISS			; (parece que no lo toca nada)
.dw C_PORTM
.dw C_SMPOS
.dw C_ORPOS
.dw C_VIBRT
.dw C_NOP
.dw C_NOP
.dw C_ENGLS
.dw C_DELAY
.dw C_NOP
.dw C_NOP
.dw C_NOP
.dw C_NOP
.dw C_NOP
.dw C_NOP





CHREGS:
  xor  a
	ld   (#_AYREGS+AR_AmplC),a
	bit   0,CHNPRM_Flags(ix)
	push  hl
	jp   z,CH_EXIT
	ld	 (#_PT3_CSP),sp
	ld   l,CHNPRM_OrnPtr(ix)
	ld   h,CHNPRM_OrnPtr+1(ix)
	ld   sp,hl
	pop  de
	ld   h,a
	ld   a,CHNPRM_PsInOr(ix)
	ld   l,a
	add  hl,sp
	inc  a
	cp   d
	jr   c,CH_ORPS
	ld   a,e
CH_ORPS:
  ld   CHNPRM_PsInOr(ix),a
	ld   a,CHNPRM_Note(ix)
	add  a,(hl)
	jp   p,CH_NTP
	xor  a
CH_NTP:
  cp   #96
	jr   c,CH_NOK
	ld   a,#95
CH_NOK:
  add  a,a
	ex   af,af
	ld   l,CHNPRM_SamPtr(ix)
	ld   h,CHNPRM_SamPtr+1(ix)
	ld   sp,hl
	pop  de
	ld   h,#0
	ld   a,CHNPRM_PsInSm(ix)
	ld   b,a
	add  a,a
	add  a,a
	ld   l,a
	add  hl,sp
	ld   sp,hl
	ld   a,b
  inc  a
	cp   d
	jr   c,CH_SMPS
	ld   a,e
CH_SMPS:
  ld   CHNPRM_PsInSm(ix),a
	pop  bc
	pop  hl
	ld   e,CHNPRM_TnAcc(ix)
	ld   d,CHNPRM_TnAcc+1(ix)
	add  hl,de
	bit  6,b
	jr   z,CH_NOAC
	ld   CHNPRM_TnAcc(ix),l
	ld   CHNPRM_TnAcc+1(ix),h
CH_NOAC:
  ex   de,hl
	ex   af,af
	ld   l,a
	ld   h,#0
	ld   sp,#_pt3_note_table
	add  hl,sp
	ld   sp,hl
	pop  hl
	add  hl,de
	ld   e,CHNPRM_CrTnSl(ix)
	ld   d,CHNPRM_CrTnSl+1(ix)
	add  hl,de
	ld	 sp,(#_PT3_CSP)
	ex   (sp),hl
	xor  a
	or   CHNPRM_TSlCnt(ix)
	jr   z,CH_AMP
	dec  CHNPRM_TSlCnt(ix)
	jr   nz,CH_AMP
	ld   a,CHNPRM_TnSlDl(ix)
	ld   CHNPRM_TSlCnt(ix),a
	ld   l,CHNPRM_TSlStp(ix)
	ld   h,CHNPRM_TSlStp+1(ix)
	ld   a,h
	add  hl,de
	ld   CHNPRM_CrTnSl(ix),l
	ld   CHNPRM_CrTnSl+1(ix),h
	bit  2,CHNPRM_Flags(ix)
	jr   nz,CH_AMP
	ld   e,CHNPRM_TnDelt(ix)
	ld   d,CHNPRM_TnDelt+1(ix)
	and  a
	jr   z,CH_STPP
	ex   de,hl
CH_STPP:
  sbc  hl,de
	jp   m,CH_AMP
	ld   a,CHNPRM_SlToNt(ix)
	ld   CHNPRM_Note(ix),a
	xor  a
	ld   CHNPRM_TSlCnt(ix),a
	ld   CHNPRM_CrTnSl(ix),a
	ld   CHNPRM_CrTnSl+1(ix),a
CH_AMP:
  ld   a,CHNPRM_CrAmSl(ix)
	bit  7,c
	jr   z,CH_NOAM
	bit  6,c
	jr   z,CH_AMIN
	cp   #15
	jr   z,CH_NOAM
	inc  a
	jr   CH_SVAM
CH_AMIN:
  cp   #-15
	jr   z,CH_NOAM
	dec  a
CH_SVAM:
  ld   CHNPRM_CrAmSl(ix),a
CH_NOAM:
  ld   l,a
	ld   a,B
	and  #15
	add  a,l
	jp   p,CH_APOS
	xor  a
CH_APOS:
  cp   #16
	jr   c,CH_VOL
	ld   a,#15
CH_VOL:
  or   CHNPRM_Volume(ix)
	ld   l,a
	ld   h,#0
	ld   de,#_AYREGS  ;_VT_
	add  hl,de
	ld   a,(hl)
CH_ENV:
  bit  0,c
	jr   nz,CH_NOEN
	or   CHNPRM_Env_En(ix)
CH_NOEN:
  ld   (#_AYREGS+AR_AmplC),a
	bit  7,B
	ld   a,c
	jr   Z,NO_ENSL
	rla
	rla
	sra  A
	sra  A
	sra  A
	add  a,CHNPRM_CrEnSl(ix) ;SEE COMMENT BELOW
	bit  5,B
	jr   Z,NO_ENAC
	ld   CHNPRM_CrEnSl(ix),a
NO_ENAC:
	ld	 hl,#_PT3_AddToEn
	add  a,(hl) ;BUG IN PT3 - NEED WORD HERE.
		   ;FIX IT IN NEXT VERSION?
	ld   (hl),a
	jr   CH_MIX
NO_ENSL:
  rra
	add  a,CHNPRM_CrNsSl(ix)
	ld   (#_AddToNs),a
	bit  5,B
	jr   Z,CH_MIX
	ld   CHNPRM_CrNsSl(ix),a
CH_MIX:
  ld   a,B
	rra
	and  #0x48
CH_EXIT:
  ld   hl,#_AYREGS+AR_Mixer
	or   (hl)
	rrca
	ld   (hl),a
	pop  hl
	xor  a
	or   CHNPRM_COnOff(ix)
	ret z
	dec  CHNPRM_COnOff(ix)
	ret nz
	xor  CHNPRM_Flags(ix)
	ld   CHNPRM_Flags(ix),a
	rra
	ld   a,CHNPRM_OnOffD(ix)
	jr   c,CH_ONDL
	ld   a,CHNPRM_OffOnD(ix)
CH_ONDL:
  ld   CHNPRM_COnOff(ix),a
	ret

EMPTYSAMORN:
  .db 0,1,0,0x90

__endasm;
}
