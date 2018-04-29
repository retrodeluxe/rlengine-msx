#ifndef _MUSIC_H
#define _MUSIC_H

#include <stddef.h>

/*
 * Trilo Tracker PSG Replayer Copyright 2016 Richard Cornelisse
 *
 * Ported to SDCC for RDL Engine by Enric Martin Geijo 2018
 */

#define TRACK_Instrument		0-17
#define TRACK_Waveform		    1-17
#define TRACK_Command		    2-17
#define TRACK_MacroPointer	    3-17
#define TRACK_MacroStart		5-17
#define TRACK_MacroRestart 	    7-17
#define TRACK_Note			    8-17
#define TRACK_Volume		    9-17
#define TRACK_Flags			    10-17
	// 0 = note trigger
	// 1 = note active
	// 4 = morph active; for SCC when 1 then waveform is following morph buffer
	// 3 = command trigger
	// 2 = envelope trigger
	// 5 = instrument trigger
	// 6 = waveform trigger
	// 7 = PSG/SCC
#define TRACK_empty			    11-17		// needed for pushing 0 at note start
#define TRACK_ToneAdd		    12-17		// reset after note set
#define TRACK_VolumeAdd		    14-17		// reset after note set
#define TRACK_Noise			    15-17		// reset after note set
#define TRACK_cmd_VolumeAdd	    16-17		// reset after note set
#define TRACK_cmd_ToneSlideAdd	17-17		// reset after note set
#define TRACK_cmd_ToneAdd		19-17		// reset after note set

#define TRACK_cmd_detune		21-17
#define TRACK_cmd_0			    22-17
#define TRACK_cmd_1			    23-17
#define TRACK_cmd_2			    24-17
#define TRACK_cmd_3			    25-17
#define TRACK_cmd_4_depth		26-17
#define TRACK_cmd_4_step		27-17
#define TRACK_cmd_NoteAdd		28-17		// x reset after note set
#define TRACK_cmd_A			    29-17
#define TRACK_cmd_B			    30-17
#define TRACK_cmd_E			    31-17
#define TRACK_Timer			    32-17		// used for timing by all cmd's
#define TRACK_Step			    33-17		// only for VIBRATO???
#define TRACK_Delay			    34-17		// rows to wait till next data
#define TRACK_prevDelay		    35-17
#define TRACK_Retrig		    36-17		// rows to retrigger command
#define TRACK_cmd_A_add		    37-17	    //<< Still in use???

#define TRACK_REC_SIZE		    38


#define B_TRGNOT			0			// note trigger
#define B_ACTNOT			1			// note active
#define B_TRGENV			2			// envelope trigger
#define B_TRGCMD			3			// command active
#define B_ACTMOR			4		    // morph active
#define B_TRGINS			5			// instrument trigger
#define B_TRGWAV			6			// waveform trigger
#define B_PSGSCC			7			// chip type (PSG or SCC)

struct replayer {
	uint8_t trigger;       // trigger byte.
	uint8_t main_psg_vol;    // volume mixer for PSG SCC balance
	uint8_t *song;         // pointer to song data
	uint8_t *wave;         // pointer to waveform data
	uint8_t *instr;        // pointer to instrument data
	uint8_t *order;        // pointer to the order track list pointers
	uint8_t speed;         // speed to replay (got from song)
	uint8_t speed_subtimer;
    uint8_t speed_timer;
    uint8_t mode;           // replayer status
        #define REPLAYER_NOSOUND    0
        #define REPLAYER_PLAY       1
    uint8_t prev_note;      // previous note played
    uint8_t main_vol;       // volume corrections
    uint8_t *vibrato_table; // pointer to vibrato table
    uint8_t *tone_table;    // pointer to tone table (affected by transpose)
};

void music_init(uint16_t* song);
void music_play(void);
void music_mute(void);

#endif
