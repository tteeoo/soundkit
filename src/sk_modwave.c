#include <math.h>
#include <stdlib.h>

#include "sk_modwave.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832
#endif

//
// Helper wave functions
//
static float modwave_sinf(double time, double frequency, double phase, double amplitude) {
	return (float)(sin(2 * M_PI * frequency * time + phase) * amplitude);
}

static float modwave_sawtooth(double time, double frequency, double phase, double amplitude) {
	time *= frequency;
	time += phase;

	double f = time - (ma_int64)time;
	double r;

	r = 2 * (f - 0.5);

	return (float)(r * amplitude);
}

static float modwave_square(double time, double frequency, double phase, double amplitude) {
	time *= frequency;
	time += phase;

	double f = time - (ma_int64)time;
	double r;

	if (f < 0.5) {
		r = amplitude;
	} else {
		r = -amplitude;
	}

	return (float)r;
}

static float modwave_triangle(double time, double frequency, double phase, double amplitude) {
	time *= frequency;
	time += phase;

	double f = time - (ma_int64)time;
	double r;

	r = 2 * fabs(2 * (f - 0.5)) - 1;

	return (float)(r * amplitude);
}

static float modwave(ma_waveform_type type, double time, double frequency, double phase, double amplitude) {
	switch (type) {
		case ma_waveform_type_sine:
			return modwave_sinf(time, frequency, phase, amplitude);
		case ma_waveform_type_square:
			return modwave_square(time, frequency, phase, amplitude);
		case ma_waveform_type_sawtooth:
			return modwave_sawtooth(time, frequency, phase, amplitude);
		case ma_waveform_type_triangle:
			return modwave_triangle(time, frequency, phase, amplitude);
		default:
			return 0;
	}
}


//
// Direct modwave function
//
void sk_modwave_read_pcm_frames(sk_modwave* pModWave, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {

	float pModFrames[pModWave->config.channels * frameCount];
	if (pModWave->config.mod_source != NULL) {
		ma_data_source_read_pcm_frames(pModWave->config.mod_source, pModFrames, frameCount, NULL);
	}

	float* pFramesOutF32 = (float*)pFramesOut;
	for (ma_uint64 iFrame = 0; iFrame < frameCount; iFrame += 1) {
		if (pModWave->config.mod_source == NULL) {
			float s = modwave_sinf(pModWave->time, pModWave->config.cfrequency,
				modwave_sinf(pModWave->time, pModWave->config.mfrequency, 0, pModWave->config.mamplitude), pModWave->config.camplitude);
			for (ma_uint64 iChannel = 0; iChannel < pModWave->config.channels; iChannel += 1)
				pFramesOutF32[iFrame*pModWave->config.channels + iChannel] = s;
		} else {
			for (ma_uint64 iChannel = 0; iChannel < pModWave->config.channels; iChannel += 1) {
				pModWave->phaseAccum[iChannel] += pModFrames[iFrame*pModWave->config.channels + iChannel];
				pFramesOutF32[iFrame*pModWave->config.channels + iChannel] =
					modwave(pModWave->config.type, pModWave->time, pModWave->config.cfrequency,
						(pModWave->config.mamplitude / 8.1169) * pModWave->phaseAccum[iChannel], pModWave->config.camplitude);
			}
		}

		pModWave->time += 1.0 / pModWave->config.sample_rate;
	}

	if (pFramesRead != NULL)
		*pFramesRead = frameCount;
}

void sk_modwave_seek_to_pcm_frame(sk_modwave* pModWave, ma_uint64 frameIndex) {
	pModWave->time = (double)frameIndex / (double)pModWave->config.sample_rate;
}


//
// vtable bindings
//
static ma_result sk_modwave_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
	sk_modwave_read_pcm_frames((sk_modwave*)pDataSource, pFramesOut, frameCount, pFramesRead);
	return MA_SUCCESS;
}

static ma_result sk_modwave_on_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
	sk_modwave_seek_to_pcm_frame((sk_modwave*)pDataSource, frameIndex);
	return MA_SUCCESS;
}

static ma_result sk_modwave_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {

	sk_modwave* pModWave = (sk_modwave*)pDataSource;

	*pFormat = ma_format_f32;
	*pChannels = pModWave->config.channels;
	*pSampleRate = pModWave->config.sample_rate;
	ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pModWave->config.channels);

	return MA_SUCCESS;
}

static ma_result sk_modwave_on_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor) {

	sk_modwave* pModWave = (sk_modwave*)pDataSource;
	*pCursor = (ma_uint64)(pModWave->time * pModWave->config.sample_rate);
	return MA_SUCCESS;
}

static ma_data_source_vtable g_sk_modwave_vtable = {
	sk_modwave_on_read,
	sk_modwave_on_seek,
	sk_modwave_on_get_data_format,
	sk_modwave_on_get_cursor,
	NULL, // No get_length
	NULL, // No set_looping
	0
};

//
// struct init/uninit methods
//
sk_modwave_config sk_modwave_config_init(ma_uint32 channels, ma_uint32 sample_rate, ma_waveform_type type, double camplitude, double mamplitude, double cfrequency, double mfrequency, ma_data_source* mod_source) {

	sk_modwave_config config;

	config.channels = channels;
	config.sample_rate = sample_rate;
	config.type = type;
	config.camplitude = camplitude;
	config.mamplitude = mamplitude;
	config.cfrequency = cfrequency;
	config.mfrequency = mfrequency;
	config.mod_source = mod_source;

	return config;
}

ma_result sk_modwave_init(sk_modwave_config* pConfig, sk_modwave* pModWave) {

	ma_result result;
	ma_data_source_config baseConfig;

	baseConfig = ma_data_source_config_init();
	baseConfig.vtable = &g_sk_modwave_vtable;

	result = ma_data_source_init(&baseConfig, &pModWave->base);
	if (result != MA_SUCCESS)
		return result;

	if (pModWave == NULL)
		return MA_INVALID_ARGS;

	pModWave->config = *pConfig;
	pModWave->time = 0;

	pModWave->phaseAccum = (float*)calloc(pModWave->config.channels, sizeof(float));

	return MA_SUCCESS;
}

void sk_modwave_uninit(sk_modwave* pModWave) {

	if (pModWave == NULL)
		return;

	free(pModWave->phaseAccum);

	ma_data_source_uninit(&pModWave->base);
}
