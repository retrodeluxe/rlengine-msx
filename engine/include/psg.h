/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#ifndef _PSG_H_
#define _PSG_H_

struct ay_reg_map {
	uint8_t A_tone_fine;
	uint8_t A_tone_coarse;
	uint8_t B_tone_fine;
	uint8_t B_tone_coarse;
	uint8_t C_tone_fine;
	uint8_t C_tone_coarse;
	uint8_t Noise_period;
	uint8_t Mixer_Ctl;
	uint8_t A_Amp_Ctl;
	uint8_t B_Amp_Ctl;
	uint8_t C_Amp_Ctl;
	uint8_t Env_period_fine;
	uint8_t Env_period_coarse;
	uint8_t Env_shape;
};

extern void psg_write(uint8_t reg, uint8_t val);
extern void psg_set_all(struct ay_reg_map *regs);
extern void psg_set_tone(unsigned int period, uint8_t chan);
extern void psg_set_noise(uint8_t period);
extern void psg_set_mixer(uint8_t mixval);
extern void psg_set_vol(uint8_t chan, uint8_t vol);
extern void psg_set_envelope(unsigned int period, uint8_t shape);

#endif
