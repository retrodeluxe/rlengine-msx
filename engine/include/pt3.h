/*
 *  SDCC Vortex Tracker II PT3 player for MSX
 *
 * - Vortex Tracker II v1.0 PT3 player for ZX Spectrum by S.V.Bulba
 *	 <vorobey@mail.khstu.ru> http://bulba.at.kz
 *  - (09-Jan-05) Adapted to MSX by Alfonso D. C. aka Dioniso
 *<dioniso072@yahoo.es>
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

#ifndef __PT3_PLAYER_H__
#define __PT3_PLAYER_H__

#include "msx.h"

#define AY0index 0xA0
#define AY0write 0xA1
#define AY0read 0xA2

#define CHNPRM_PsInOr 0
#define CHNPRM_PsInSm 1
#define CHNPRM_CrAmSl 2
#define CHNPRM_CrNsSl 3
#define CHNPRM_CrEnSl 4
#define CHNPRM_TSlCnt 5
#define CHNPRM_CrTnSl 6
#define CHNPRM_TnAcc 8
#define CHNPRM_COnOff 10
#define CHNPRM_OnOffD 11
#define CHNPRM_OffOnD 12
#define CHNPRM_OrnPtr 13
#define CHNPRM_SamPtr 15
#define CHNPRM_NNtSkp 17
#define CHNPRM_Note 18
#define CHNPRM_SlToNt 19
#define CHNPRM_Env_En 20
#define CHNPRM_Flags 21
#define CHNPRM_TnSlDl 22
#define CHNPRM_TSlStp 23
#define CHNPRM_TnDelt 25
#define CHNPRM_NtSkCn 27
#define CHNPRM_Volume 28
#define CHNPRM_Size 29

#define AR_TonA 0
#define AR_TonB 2
#define AR_TonC 4
#define AR_Noise 6
#define AR_Mixer 7
#define AR_AmplA 8
#define AR_AmplB 9
#define AR_AmplC 10
#define AR_Env 11
#define AR_EnvTp 13

void pt3_init(uint8_t *song, uint8_t loop);
void pt3_play() __nonbanked;
void pt3_decode() __nonbanked;
void pt3_mute();
void pt3_init_notes(uint8_t *note_table);

#endif
