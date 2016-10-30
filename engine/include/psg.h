#ifndef _PSG_H_
#define _PSG_H_

struct ay_reg_map {
	byte A_tone_fine;
	byte A_tone_coarse;
	byte B_tone_fine;
	byte B_tone_coarse;
	byte C_tone_fine;
	byte C_tone_coarse;
	byte Noise_period;
	byte Mixer_Ctl;
	byte A_Amp_Ctl;
	byte B_Amp_Ctl;
	byte C_Amp_Ctl;
	byte Env_period_fine;
	byte Env_period_coarse;
	byte Env_shape;
};

extern void psg_set_all(struct ay_reg_map *regs);
extern void psg_set_tone(unsigned int period, byte chan);
extern void psg_set_noise(byte period);
extern void psg_set_mixer(byte mixval);
extern void psg_set_vol(byte chan, byte vol);
extern void psg_set_envelope(unsigned int period, byte shape);

#endif
