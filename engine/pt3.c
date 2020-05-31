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

#pragma CODE_PAGE 2

char ChanA[29]; //CHNPRM_Size
char ChanB[29];
char ChanC[29];

char DelyCnt;
unsigned int CurESld;
char CurEDel;

//Ns_Base_AddToNs:
char Ns_Base;
char AddToNs;

//AYREGS::
char VT_[14];
char AYREGS[14];
unsigned int EnvBase;
char VAR0END[240];

/* --- Workarea --- (apunta a RAM que estaba antes en codigo automodificable)
 -El byte de estado en SETUP deberia ser algo asi (CH enable/disable no esta aun)
|EP|0|0|0|CH3|CH2|CH1|LP|

LP: Loop enable/disable. A 1 si queremos que el tema suene solo una vez.
EP: End point. A 1 cada vez que el tema acaba.
CH1-CH3: Channel enable/disable. A 1 si no queremos que suene el canal. (AUN  NO VA!!)
*/

char PT3_SETUP; /* set bit0 to 1, if you want to play without looping
				           bit7 is set each time, when loop point is passed           */
unsigned int PT3_MODADDR;	 //direccion datos canci�n
unsigned int PT3_CrPsPtr;  //POSICION CURSOR EN PATTERN
unsigned int PT3_SAMPTRS;  //sample info?
unsigned int PT3_OrnPtrs;  //Ornament pattern

unsigned int PT3_PDSP;     //pilasave
unsigned int PT3_CSP;      //pilsave2
unsigned int PT3_PSP;      //pilsave3

char PT3_PrNote;
unsigned int PT3_PrSlide;

unsigned int PT3_AdInPtA;  //play data pattern
unsigned int PT3_AdInPtB;  //play data
unsigned int PT3_AdInPtC;  //play data

unsigned int PT3_LPosPtr;  //Position Ptr?
unsigned int PT3_PatsPtr;  //Pat Ptr

char PT3_Delay;            //delay
char PT3_AddToEn;          //Envelope data (No cal ya que no usa Envs??)
char PT3_Env_Del;          //Envelope data (idem)
unsigned int PT3_ESldAdd;  //Envelope data (idem)


char NoteTable[192];       //Note table

/* -----------------------------------------------------------------------------
PT3Mute
Silence the PSG.
----------------------------------------------------------------------------- */
void pt3_mute() __naked
{
	__asm
MUTE:
	XOR  A
	LD   (#_AYREGS+AR_AmplA),A
	LD   (#_AYREGS+AR_AmplB),A
	LD   (#_AYREGS+AR_AmplC),A
	JP   _pt3_play

	__endasm;
}
// ----------------------------------------------------------------------------- END PT3Stop




/* -----------------------------------------------------------------------------
PT3Init
(unsigned int) Song data address
(char) Loop - 0=off ; 1=on  (false = 0, true = 1));
----------------------------------------------------------------------------- */
void pt3_init(uint8_t *song,uint8_t loop) __naked
{
	song;
	loop;

	__asm

	push IX
	ld   IX,#0
	add  IX,SP

	ld   HL,#_PT3_SETUP
	ld   A,6(IX)
	or   A
	jr   NZ,SongLoop
	set  0,(HL)  ;not loop
	jr   initSong

SongLoop:
	res  0,(HL)  ;loop

initSong:
	ld   L,4(IX)
	ld   H,5(IX)
	call playerINIT
	pop  IX
	ret

; HL - AddressOfModule
playerINIT::
	LD   (#_PT3_MODADDR),HL
	PUSH HL

	LD   DE,#100
	ADD  HL,DE

	LD   A,(HL)            ;+100 = 1B Delay
	LD   (#_PT3_Delay),A

	PUSH HL
	POP  IX                 ;<<-- IX = _PT3_MODADDR + 100

  ADD  HL,DE
	LD   (#_PT3_CrPsPtr),HL  ;+200 = Cr Ps Ptr data

	LD   E,2(IX)
	ADD  HL,DE
	INC  HL
	LD   (#_PT3_LPosPtr),HL  ;

  POP  DE                 ;<<-- DE = _PT3_MODADDR

	LD   L,3(IX)
	LD   H,4(IX)
  ADD  HL,DE
	LD   (#_PT3_PatsPtr),HL

	LD   HL,#169
	ADD  HL,DE
	LD   (#_PT3_OrnPtrs),HL

	LD   HL,#105
	ADD  HL,DE
	LD   (#_PT3_SAMPTRS),HL

	LD   HL,#_PT3_SETUP
	RES  7,(HL)


	; Create Volume Table for Vortex Tracker II/PT3.5
	; (c) Ivan Roshin, adapted by SapphiRe ---
	ld	HL,#0x11
	ld	D,H
	ld  E,H
	ld  IX,#_VAR0END  ;_VT_+16
	ld  B,#15
INITV1:
	push HL
	add  HL,DE
	ex   DE,HL
	sbc  HL,HL
	ld   C,B
	ld   B,#16
INITV2:
	ld   A,L
	rla
	ld   A,H
	adc  A,#0
	ld   (IX),A
	inc  IX
	add  HL,DE
	djnz INITV2
	pop  HL
	ld   A,E
	cp   #0x77
	jr   NZ,INITV3
	inc  E
INITV3:
	ld   B,C
	djnz INITV1

; --- INITIALIZE PT3 VARIABLES ---
	xor	 A
	LD   HL,#_ChanA     ;VARS
	LD   (HL),A
	LD   DE,#_ChanA+1   ;VARS+1
	LD   BC,#_VAR0END - _ChanA -1  ;VARS -1
	LDIR

	INC  A
	LD   (#_DelyCnt),A
	LD   HL,#0xF001	;H - CHNPRM_Volume, L - CHNPRM_NtSkCn
	LD   (#_ChanA+CHNPRM_NtSkCn),HL
	LD   (#_ChanB+CHNPRM_NtSkCn),HL
	LD   (#_ChanC+CHNPRM_NtSkCn),HL

	LD   HL,#EMPTYSAMORN
	LD   (#_PT3_AdInPtA),HL ;ptr to zero  ; # chg
	LD   (#_ChanA+CHNPRM_OrnPtr),HL ;ornament 0 is "0,1,0"
	LD   (#_ChanB+CHNPRM_OrnPtr),HL ;in all versions from
	LD   (#_ChanC+CHNPRM_OrnPtr),HL ;3.xx to 3.6x and VTII

	LD   (#_ChanA+CHNPRM_SamPtr),HL ;S1 There is no default
	LD   (#_ChanB+CHNPRM_SamPtr),HL ;S2 sample in PT3, so, you
	LD   (#_ChanC+CHNPRM_SamPtr),HL ;S3 can comment S1,2,3; see
				    ;also EMPTYSAMORN comment
	ret

	__endasm;
}

void pt3_init_notes(uint8_t *note_table)
{
	sys_memcpy(NoteTable, note_table, 96*2);
}

void pt3_play() __naked __nonbanked
{
	__asm

	ld   A,(#_AYREGS+AR_Mixer)
	AND  #0b00111111
	ld   B,A

	ld   A,#AR_Mixer
	out  (#AY0index),A
	in   A,(#AY0read)
	and	 #0b11000000	; Mascara para coger dos bits de joys
	or	 B		        ; A�ado Byte de B

	ld   (#_AYREGS+AR_Mixer),A

	XOR  A

	ld   C,#AY0index
	ld   HL,#_AYREGS
LOUT:
	OUT  (C),A
	INC  C
	OUTI
	DEC  C
	INC  A
	CP   #13
	JR   NZ,LOUT
	RET

__endasm;
}

void pt3_decode() __naked __nonbanked
{
	__asm

	XOR  A
	LD   (#_PT3_AddToEn),A
	LD   (#_AYREGS+AR_Mixer),A
	DEC  A
	LD   (#_AYREGS+AR_EnvTp),A
	LD   HL,#_DelyCnt
	DEC  (HL)
	JP   NZ,PL2
	LD   HL,#_ChanA+CHNPRM_NtSkCn
	DEC  (HL)
	JR   NZ,PL1B
	ld	 BC,(#_PT3_AdInPtA)
	LD   A,(BC)
	AND  A
	JR   NZ,PL1A
	LD   D,A
	LD   (#_Ns_Base),A
	LD   HL,(#_PT3_CrPsPtr)
	INC  HL
	LD   A,(HL)
	INC  A
	JR   NZ,PLNLP
	CALL CHECKLP
	ld	 HL,(#_PT3_LPosPtr)
	LD   A,(HL)
	INC  A
PLNLP:
	LD   (#_PT3_CrPsPtr),HL
	DEC  A
	ADD  A,A
	LD   E,A
	RL   D
	ld   HL,(#_PT3_PatsPtr)
	ADD  HL,DE
	LD   DE,(#_PT3_MODADDR)
	ld   (#_PT3_PSP),SP
	LD   SP,HL
	POP  HL
	ADD  HL,DE
	LD   B,H
	LD   C,L
	POP  HL
	ADD  HL,DE
	LD   (#_PT3_AdInPtB),HL
	POP  HL
	ADD  HL,DE
	LD   (#_PT3_AdInPtC),HL
	ld   SP,(#_PT3_PSP)

PL1A:
	LD   IX,#_ChanA+12
	CALL PTDECOD
	LD   (#_PT3_AdInPtA),BC

PL1B:
	LD   HL,#_ChanB+CHNPRM_NtSkCn
	DEC  (HL)
	JR   NZ,PL1C
	LD   IX,#_ChanB+12
	ld   BC,(#_PT3_AdInPtB)
	CALL PTDECOD
	LD   (#_PT3_AdInPtB),BC

PL1C:
	LD   HL,#_ChanC+CHNPRM_NtSkCn
	DEC  (HL)
	JR   NZ,PL1D
	LD   IX,#_ChanC+12
	ld   BC,(#_PT3_AdInPtC)
	CALL PTDECOD
	LD   (#_PT3_AdInPtC),BC

PL1D:
	ld   A,(#_PT3_Delay)
	ld   (#_DelyCnt),A

PL2:
	LD   IX,#_ChanA
	LD   HL,(#_AYREGS+AR_TonA)
	CALL CHREGS
	LD   (#_AYREGS+AR_TonA),HL
	LD   A,(#_AYREGS+AR_AmplC)
	LD   (#_AYREGS+AR_AmplA),A
	LD   IX,#_ChanB
	LD   HL,(#_AYREGS+AR_TonB)
	CALL CHREGS
	LD   (#_AYREGS+AR_TonB),HL
	LD   A,(#_AYREGS+AR_AmplC)
	LD   (#_AYREGS+AR_AmplB),A
	LD   IX,#_ChanC
	LD   HL,(#_AYREGS+AR_TonC)
	CALL CHREGS
	LD   (#_AYREGS+AR_TonC),HL

	LD   HL,(#_Ns_Base)    ;Ns_Base_AddToNs
	LD   A,H
	ADD  A,L
	LD   (#_AYREGS+AR_Noise),A

	ld   A,(#_PT3_AddToEn)
	LD   E,A
	ADD  A,A
	SBC  A,A
	LD   D,A
	LD   HL,(#_EnvBase)
	ADD  HL,DE
	LD   DE,(#_CurESld)
	ADD  HL,DE
	LD  (#_AYREGS+AR_Env),HL

	XOR  A
	LD   HL,#_CurEDel
	OR   (HL)

	RET  Z
	DEC  (HL)
	RET  NZ
	LD   A,(#_PT3_Env_Del)
	LD   (HL),A
	LD   HL,(#_PT3_ESldAdd)
	ADD  HL,DE
	LD   (#_CurESld),HL
	RET

CHECKLP:
	LD   HL,#_PT3_SETUP
	SET  7,(HL)   ; -------------------------------------------------------------- <<< ????
	BIT  0,(HL)   ;loop bit
	RET  Z

;=1 - No loop
	POP  HL
	LD   HL,#_DelyCnt
	INC  (HL)
	LD   HL,#_ChanA+CHNPRM_NtSkCn
	INC  (HL)
	JP   MUTE

PD_OrSm:
	LD   -12+CHNPRM_Env_En(IX),#0
	CALL SETORN
	LD   A,(BC)
	INC  BC
	RRCA

PD_SAM:
	ADD  A,A
PD_SAM_:
	LD   E,A
	LD   D,#0
	ld	 HL,(#_PT3_SAMPTRS)
	ADD  HL,DE
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	ld	 HL,(#_PT3_MODADDR)
	ADD  HL,DE
	LD   -12+CHNPRM_SamPtr(IX),L
	LD   -12+CHNPRM_SamPtr+1(IX),H
	JR   PD_LOOP

PD_VOL:
	RLCA
	RLCA
	RLCA
	RLCA
	LD   -12+CHNPRM_Volume(IX),A
	JR   PD_LP2

PD_EOff:
	LD   -12+CHNPRM_Env_En(IX),A
	LD   -12+CHNPRM_PsInOr(IX),A
	JR   PD_LP2

PD_SorE:
	DEC  A
	JR   NZ,PD_ENV
	LD   A,(BC)
	INC  BC
	LD   -12+CHNPRM_NNtSkp(IX),A
	JR   PD_LP2

PD_ENV:
	CALL SETENV
	JR   PD_LP2

PD_ORN:
	CALL SETORN
	JR   PD_LOOP

PD_ESAM:
	LD   -12+CHNPRM_Env_En(IX),A
	LD   -12+CHNPRM_PsInOr(IX),A
	CALL NZ,SETENV
	LD   A,(BC)
	INC  BC
	JR   PD_SAM_

PTDECOD:
	LD   A,-12+CHNPRM_Note(IX)
	LD   (#_PT3_PrNote),A           ;LD   (#PrNote+1),A
	LD   L,CHNPRM_CrTnSl-12(IX)
	LD   H,CHNPRM_CrTnSl+1-12(IX)
	LD  (#_PT3_PrSlide),HL

PD_LOOP:
	ld   DE,#0x2010
PD_LP2:
	ld   A,(BC)
	inc  BC
	ADD  A,E
	JR   C,PD_OrSm
	ADD  A,D
	JR   Z,PD_FIN
	JR   C,PD_SAM
	ADD  A,E
	JR   Z,PD_REL
	JR   C,PD_VOL
	ADD  A,E
	JR   Z,PD_EOff
	JR   C,PD_SorE
	ADD  A,#96
	JR   C,PD_NOTE
	ADD  A,E
	JR   C,PD_ORN
	ADD  A,D
	JR   C,PD_NOIS
	ADD  A,E
	JR   C,PD_ESAM
	ADD  A,A
	LD   E,A

  LD   HL,#(SPCCOMS + 0xDF20)  ;LD HL,((SPCCOMS+$DF20) % 65536)
;	PUSH DE
;	LD   DE,#0xDF20
;	LD   HL,#SPCCOMS
;	ADD  HL,DE
;	POP  DE

	ADD  HL,DE
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	PUSH DE

	JR   PD_LOOP

PD_NOIS:
  LD   (#_Ns_Base),A
	JR   PD_LP2

PD_REL:
  RES  0,-12+CHNPRM_Flags(IX)
	JR   PD_RES

PD_NOTE:
  ld   -12+CHNPRM_Note(IX),A
	SET  0,-12+CHNPRM_Flags(IX)
	XOR  A

PD_RES:
	ld	 (#_PT3_PDSP),SP
	LD   SP,IX
	LD   H,A
	LD   L,A
	PUSH HL
	PUSH HL
	PUSH HL
	PUSH HL
	PUSH HL
	PUSH HL
	ld	SP,(#_PT3_PDSP)

PD_FIN:
  ld   A,-12+CHNPRM_NNtSkp(IX)
	ld   -12+CHNPRM_NtSkCn(IX),A
	ret

C_PORTM:
  RES  2,-12+CHNPRM_Flags(IX)
	LD   A,(BC)
	INC  BC
;SKIP PRECALCULATED TONE DELTA (BECAUSE
;CANNOT BE RIGHT AFTER PT3 COMPILATION)
	INC  BC
	INC  BC
	LD   -12+CHNPRM_TnSlDl(IX),A
	LD   -12+CHNPRM_TSlCnt(IX),A
	LD   DE,#_NoteTable
	LD   A,-12+CHNPRM_Note(IX)
	LD   -12+CHNPRM_SlToNt(IX),A
	ADD  A,A
	LD   L,A
	LD   H,#0
	ADD  HL,DE
	LD   A,(HL)
	INC  HL
	LD   H,(HL)
	LD   L,A
	PUSH HL
  LD   A,(#_PT3_PrNote)            ;<--- LD   A,#0x3E
	LD   -12+CHNPRM_Note(IX),A
	ADD  A,A
	LD   L,A
	LD   H,#0
	ADD  HL,DE
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	POP  HL
	SBC  HL,DE
	LD   -12+CHNPRM_TnDelt(IX),L
	LD   -12+CHNPRM_TnDelt+1(IX),H
  LD   DE,(#_PT3_PrSlide)             ;<--- change to Kun version
	LD   -12+CHNPRM_CrTnSl(IX),E       ;<---
	LD   -12+CHNPRM_CrTnSl+1(IX),D     ;<---
  LD   A,(BC) ;SIGNED TONE STEP
	INC  BC
	EX   AF,AF
	LD   A,(BC)
	INC  BC
	AND  A
	JR   Z,NOSIG
	EX   DE,HL
NOSIG:
  SBC  HL,DE
	JP   P,SET_STP
	CPL
	EX   AF,AF
	NEG
	EX   AF,AF
SET_STP:
  LD   -12+CHNPRM_TSlStp+1(IX),A
	EX   AF,AF
	ld   -12+CHNPRM_TSlStp(IX),A
	ld   -12+CHNPRM_COnOff(IX),#0
	ret

C_GLISS:
  SET  2,-12+CHNPRM_Flags(IX)
	LD   A,(BC)
	INC  BC
	LD  -12+CHNPRM_TnSlDl(IX),A
	LD  -12+CHNPRM_TSlCnt(IX),A
	LD   A,(BC)
	INC  BC
	EX   AF,AF
	LD   A,(BC)
	INC  BC
	JR   SET_STP

C_SMPOS:
  LD   A,(BC)
	INC  BC
	LD   -12+CHNPRM_PsInSm(IX),A
	RET

C_ORPOS:
  LD   A,(BC)
	INC  BC
	LD   -12+CHNPRM_PsInOr(IX),A
	RET

C_VIBRT:
  LD   A,(BC)
	INC  BC
	LD   -12+CHNPRM_OnOffD(IX),A
	LD   -12+CHNPRM_COnOff(IX),A
	LD   A,(BC)
	INC  BC
	LD   -12+CHNPRM_OffOnD(IX),A
	XOR  A
	LD   -12+CHNPRM_TSlCnt(IX),A
	LD   -12+CHNPRM_CrTnSl(IX),A
	LD   -12+CHNPRM_CrTnSl+1(IX),A
	RET

C_ENGLS:
  LD   A,(BC)
	INC  BC
	LD   (#_PT3_Env_Del),A
	LD   (#_CurEDel),A
	LD   A,(BC)
	INC  BC
	LD   L,A
	LD   A,(BC)
	INC  BC
	LD   H,A
	LD   (#_PT3_ESldAdd),HL
	RET

C_DELAY:
  LD   A,(BC)
	INC  BC
	LD   (#_PT3_Delay),A
	RET

SETENV:
  LD   -12+CHNPRM_Env_En(IX),E
	LD   (#_AYREGS+AR_EnvTp),A
	LD   A,(BC)
	INC  BC
	LD   H,A
	LD   A,(BC)
	INC  BC
	LD   L,A
	LD   (#_EnvBase),HL
	XOR  A
	LD   -12+CHNPRM_PsInOr(IX),A
	LD   (#_CurEDel),A
	LD   H,A
	LD   L,A
	LD   (#_CurESld),HL

C_NOP:
  RET

SETORN:
  ADD  A,A
	LD   E,A
	LD   D,#0
	LD   -12+CHNPRM_PsInOr(IX),D
	ld	 HL,(#_PT3_OrnPtrs)
	ADD  HL,DE
	LD   E,(HL)
	INC  HL
	LD   D,(HL)
	ld   HL,(#_PT3_MODADDR)
	ADD  HL,DE
	LD   -12+CHNPRM_OrnPtr(IX),L
	LD   -12+CHNPRM_OrnPtr+1(IX),H
	RET





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
  XOR  A
	LD   (#_AYREGS+AR_AmplC),A
	BIT   0,CHNPRM_Flags(IX)
	PUSH  HL
	JP    Z,CH_EXIT
	ld	 (#_PT3_CSP),sp
	LD   L,CHNPRM_OrnPtr(IX)
	LD   H,CHNPRM_OrnPtr+1(IX)
	LD   SP,HL
	POP  DE
	LD   H,A
	LD   A,CHNPRM_PsInOr(IX)
	LD   L,A
	ADD  HL,SP
	INC  A
	CP   D
	JR   C,CH_ORPS
	LD   A,E
CH_ORPS:
  LD   CHNPRM_PsInOr(IX),A
	LD   A,CHNPRM_Note(IX)
	ADD  A,(HL)
	JP   P,CH_NTP
	XOR  A
CH_NTP:
  CP   #96
	JR   C,CH_NOK
	LD   A,#95
CH_NOK:
  ADD  A,A
	EX   AF,AF
	LD   L,CHNPRM_SamPtr(IX)
	LD   H,CHNPRM_SamPtr+1(IX)
	LD   SP,HL
	POP  DE
	LD   H,#0
	LD   A,CHNPRM_PsInSm(IX)
	LD   B,A
	ADD  A,A
	ADD  A,A
	LD   L,A
	ADD  HL,SP
	LD   SP,HL
	LD   A,B
	INC  A
	CP   D
	JR   C,CH_SMPS
	LD   A,E
CH_SMPS:
  LD   CHNPRM_PsInSm(IX),A
	POP  BC
	POP  HL
	LD   E,CHNPRM_TnAcc(IX)
	LD   D,CHNPRM_TnAcc+1(IX)
	ADD  HL,DE
	BIT  6,B
	JR   Z,CH_NOAC
	LD   CHNPRM_TnAcc(IX),L
	LD   CHNPRM_TnAcc+1(IX),H
CH_NOAC:
  EX   DE,HL
	EX   AF,AF
	LD   L,A
	LD   H,#0
	LD   SP,#_NoteTable
	ADD  HL,SP
	LD   SP,HL
	POP  HL
	ADD  HL,DE
	LD   E,CHNPRM_CrTnSl(IX)
	LD   D,CHNPRM_CrTnSl+1(IX)
	ADD  HL,DE
	ld	 SP,(#_PT3_CSP)
	EX   (SP),HL
	XOR  A
	OR   CHNPRM_TSlCnt(IX)
	JR   Z,CH_AMP
	DEC  CHNPRM_TSlCnt(IX)
	JR   NZ,CH_AMP
	LD   A,CHNPRM_TnSlDl(IX)
	LD   CHNPRM_TSlCnt(IX),A
	LD   L,CHNPRM_TSlStp(IX)
	LD   H,CHNPRM_TSlStp+1(IX)
	LD   A,H
	ADD  HL,DE
	LD   CHNPRM_CrTnSl(IX),L
	LD   CHNPRM_CrTnSl+1(IX),H
	BIT  2,CHNPRM_Flags(IX)
	JR   NZ,CH_AMP
	LD   E,CHNPRM_TnDelt(IX)
	LD   D,CHNPRM_TnDelt+1(IX)
	AND  A
	JR   Z,CH_STPP
	EX   DE,HL
CH_STPP:
  SBC  HL,DE
	JP   M,CH_AMP
	LD   A,CHNPRM_SlToNt(IX)
	LD   CHNPRM_Note(IX),A
	XOR  A
	LD   CHNPRM_TSlCnt(IX),A
	LD   CHNPRM_CrTnSl(IX),A
	LD   CHNPRM_CrTnSl+1(IX),A
CH_AMP:
  LD   A,CHNPRM_CrAmSl(IX)
	BIT  7,C
	JR   Z,CH_NOAM
	BIT  6,C
	JR   Z,CH_AMIN
	CP   #15
	JR   Z,CH_NOAM
	INC  A
	JR   CH_SVAM
CH_AMIN:
  CP   #-15
	JR   Z,CH_NOAM
	DEC  A
CH_SVAM:
  LD   CHNPRM_CrAmSl(IX),A
CH_NOAM:
  LD   L,A
	LD   A,B
	AND  #15
	ADD  A,L
	JP   P,CH_APOS
	XOR  A
CH_APOS:
  CP   #16
	JR   C,CH_VOL
	LD   A,#15
CH_VOL:
  OR   CHNPRM_Volume(IX)
	LD   L,A
	LD   H,#0
	LD   DE,#_AYREGS  ;_VT_
	ADD  HL,DE
	LD   A,(HL)
CH_ENV:
  BIT  0,C
	JR   NZ,CH_NOEN
	OR   CHNPRM_Env_En(IX)
CH_NOEN:
  LD   (#_AYREGS+AR_AmplC),A
	BIT  7,B
	LD   A,C
	JR   Z,NO_ENSL
	RLA
	RLA
	SRA  A
	SRA  A
	SRA  A
	ADD  A,CHNPRM_CrEnSl(IX) ;SEE COMMENT BELOW
	BIT  5,B
	JR   Z,NO_ENAC
	LD   CHNPRM_CrEnSl(IX),A
NO_ENAC:
	ld	 HL,#_PT3_AddToEn
	ADD  A,(HL) ;BUG IN PT3 - NEED WORD HERE.
		   ;FIX IT IN NEXT VERSION?
	LD   (HL),A
	JR   CH_MIX
NO_ENSL:
  RRA
	ADD  A,CHNPRM_CrNsSl(IX)
	LD   (#_AddToNs),A
	BIT  5,B
	JR   Z,CH_MIX
	LD   CHNPRM_CrNsSl(IX),A
CH_MIX:
  LD   A,B
	RRA
	AND  #0x48
CH_EXIT:
  LD   HL,#_AYREGS+AR_Mixer
	OR   (HL)
	RRCA
	LD   (HL),A
	POP  HL
	XOR  A
	OR   CHNPRM_COnOff(IX)
	RET  Z
	DEC  CHNPRM_COnOff(IX)
	RET  NZ
	XOR  CHNPRM_Flags(IX)
	LD   CHNPRM_Flags(IX),A
	RRA
	LD   A,CHNPRM_OnOffD(IX)
	JR   C,CH_ONDL
	LD   A,CHNPRM_OffOnD(IX)
CH_ONDL:
  LD   CHNPRM_COnOff(IX),A
	RET



;------------------------------------------------------------------------------- DATAS

EMPTYSAMORN:
  .db 0,1,0,0x90
;delete $90 if you dont need default sample  ; # pongo el 0 aqui



; As there are four tables of notes available in Vortex Tracker,
; this must be assigned externally, copying to the space reserved in the
; variable NoteTable.

;Note table 2
;NoteTable:
;  .dw 0x0D10,0x0C55,0x0BA4,0x0AFC,0x0A5F,0x09CA,0x093D,0x08B8,0x083B,0x07C5,0x0755,0x06EC
;  .dw 0x0688,0x062A,0x05D2,0x057E,0x052F,0x04E5,0x049E,0x045C,0x041D,0x03E2,0x03AB,0x0376
;  .dw 0x0344,0x0315,0x02E9,0x02BF,0x0298,0x0272,0x024F,0x022E,0x020F,0x01F1,0x01D5,0x01BB
;  .dw 0x01A2,0x018B,0x0174,0x0160,0x014C,0x0139,0x0128,0x0117,0x0107,0x00F9,0x00EB,0x00DD
;  .dw 0x00D1,0x00C5,0x00BA,0x00B0,0x00A6,0x009D,0x0094,0x008C,0x0084,0x007C,0x0075,0x006F
;  .dw 0x0069,0x0063,0x005D,0x0058,0x0053,0x004E,0x004A,0x0046,0x0042,0x003E,0x003B,0x0037
;  .dw 0x0034,0x0031,0x002F,0x002C,0x0029,0x0027,0x0025,0x0023,0x0021,0x001F,0x001D,0x001C
;  .dw 0x001A,0x0019,0x0017,0x0016,0x0015,0x0014,0x0012,0x0011,0x0010,0x000F,0x000E,0x000D



__endasm;
}
// ----------------------------------------------------------------------------- END
