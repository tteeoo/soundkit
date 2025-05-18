#include <math.h>
#include <stdio.h>

#include "sk_adsr.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

sk_adsr_config sk_adsr_config_init(ma_uint32 channels, ma_uint32 sample_rate, float attack_time, float decay_time, float sustain_time, float sustain_coeff, float release_time, ma_bool8 exponential) {

	sk_adsr_config config;
	
	config.channels = channels;
	config.sample_rate = sample_rate;

	config.attack_time = attack_time;
	config.decay_time = decay_time;
	config.sustain_time = sustain_time;
	config.sustain_coeff = sustain_coeff;
	config.release_time = release_time;
	config.exponential = exponential;

	return config;
}

ma_result sk_adsr_init(sk_adsr_config* pConfig, sk_adsr* adsr) {
	adsr->config = *pConfig;
	adsr->state = adsr->config.attack_time == 0 ? decay : attack;
	adsr->frames = 0;
	return MA_SUCCESS;
}

ma_result sk_adsr_process_pcm_frames(sk_adsr* pADSR, void* out, const void* in, ma_uint32 count) {

	float time; 
	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < count; iFrame++) {

		time = (float)(pADSR->frames + iFrame) / (float)pADSR->config.sample_rate;
		switch (pADSR->state) {
			case attack:
				if (pADSR->config.attack_time < time)
					pADSR->state++;
				else {
					for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
						outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
							* (time / pADSR->config.attack_time);
					break;
				}
			case decay:
				if (pADSR->config.attack_time + pADSR->config.decay_time < time)
					pADSR->state++;
				else {
					if (!pADSR->config.exponential)
						for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
							outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
								* ((((time - pADSR->config.attack_time) * (1 - pADSR->config.sustain_coeff)) / -(pADSR->config.decay_time)) + 1);
					else
						for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
							outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
								* (powf((time - pADSR->config.attack_time) / pADSR->config.decay_time - 1, 2.0) * (1 - pADSR->config.sustain_coeff) + pADSR->config.sustain_coeff);
					break;
				}
			case sustain:
				// TODO: responsive mode listens for silence, sustain_time not set
				if (pADSR->config.attack_time + pADSR->config.decay_time + pADSR->config.sustain_time < time)
					pADSR->state++;
				else {
					for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
						outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
							* pADSR->config.sustain_coeff;
					break;
				}
			case release:
				// TODO: in mode above, release loops to attack when done
				if (pADSR->config.attack_time + pADSR->config.decay_time + pADSR->config.sustain_time + pADSR->config.release_time < time)
					for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
						outFloat[iFrame*pADSR->config.channels + iChannel] = 0;
					/*return -1;*/
				else if (!pADSR->config.exponential)
					for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
						outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
							* ((((time - pADSR->config.attack_time - pADSR->config.decay_time - pADSR->config.sustain_time) * pADSR->config.sustain_coeff) / -(pADSR->config.release_time)) + pADSR->config.sustain_coeff);
				else
					for (ma_uint32 iChannel = 0; iChannel < pADSR->config.channels; iChannel++)
						outFloat[iFrame*pADSR->config.channels + iChannel] = inFloat[iFrame*pADSR->config.channels + iChannel]
							* (powf((time - pADSR->config.attack_time - pADSR->config.decay_time - pADSR->config.sustain_time) / pADSR->config.release_time - 1, 2.0) * pADSR->config.sustain_coeff);
		}
	}
	pADSR->frames += count;
	return MA_SUCCESS;
}
