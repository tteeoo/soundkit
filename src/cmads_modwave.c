#include <unistd.h>

#include "../miniaudio/miniaudio.h"

// TODO: separate out header file, have Makefile use object

//
// Struct definitions
//
typedef struct {
	ma_format format;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_waveform_type type; // TODO: use mtype and ctype for waves
	double camplitude;
	double mamplitude;
	double cfrequency;
	double mfrequency;
} cmads_modwave_config;

typedef struct {
	ma_data_source_base base;
	cmads_modwave_config config;
	double time;
} cmads_modwave;


//
// Direct modwave functions
// 

// Helper wave function
static float cmads_sinf(double time, double frequency, double phase, double amplitude) {
	return (float)(ma_sind(MA_TAU_D * frequency * time + phase) * amplitude);
}

static void cmads_modwave_read_pcm_frames(cmads_modwave* pModWave, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
	ma_uint64 iFrame;
	ma_uint64 iChannel;

	MA_ASSERT(pModWave  != NULL);
	MA_ASSERT(pFramesOut != NULL);

	// TODO other formats?
	if (pModWave->config.format == ma_format_f32) {
		float* pFramesOutF32 = (float*)pFramesOut;
		for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
			float s = cmads_sinf(pModWave->time, pModWave->config.cfrequency,
				cmads_sinf(pModWave->time, pModWave->config.mfrequency, 0, pModWave->config.mamplitude), pModWave->config.camplitude);
			for (iChannel = 0; iChannel < pModWave->config.channels; iChannel += 1)
				pFramesOutF32[iFrame*pModWave->config.channels + iChannel] = s;
			// TODO manage output
			if (!isatty(1))
				printf("%f\n", s);

			pModWave->time += 1.0 / pModWave->config.sampleRate;
		}
	}
}

static void cmads_modwave_seek_to_pcm_frame(cmads_modwave* pModWave, ma_uint64 frameIndex) {
	pModWave->time = (double)frameIndex / (double)pModWave->config.sampleRate;
}


//
// vtable bindings
//
static ma_result cmads_modwave_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
	cmads_modwave_read_pcm_frames((cmads_modwave*)pDataSource, pFramesOut, frameCount, pFramesRead);
	return MA_SUCCESS;
}

static ma_result cmads_modwave_on_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
	cmads_modwave_seek_to_pcm_frame((cmads_modwave*)pDataSource, frameIndex);
	return MA_SUCCESS;
}

static ma_result cmads_modwave_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {

	cmads_modwave* pModWave = (cmads_modwave*)pDataSource;

	*pFormat = pModWave->config.format;
	*pChannels = pModWave->config.channels;
	*pSampleRate = pModWave->config.sampleRate;
	ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pModWave->config.channels);

	return MA_SUCCESS;
}

static ma_result cmads_modwave_on_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor) {

	cmads_modwave* pModWave = (cmads_modwave*)pDataSource;
	*pCursor = (ma_uint64)(pModWave->time * pModWave->config.sampleRate);
	return MA_SUCCESS;
}

static ma_data_source_vtable g_cmads_modwave_vtable = {
	cmads_modwave_on_read,
	cmads_modwave_on_seek,
	cmads_modwave_on_get_data_format,
	cmads_modwave_on_get_cursor,
	NULL, // No get_length
	NULL, // No set_looping (TODO?)
	0
};

//
// struct init/uninit methods
//
cmads_modwave_config cmads_modwave_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_waveform_type type, double camplitude, double mamplitude, double cfrequency, double mfrequency) {

	cmads_modwave_config config;
	MA_ZERO_OBJECT(&config);

	config.format = format;
	config.channels = channels;
	config.sampleRate = sampleRate;
	config.type = type;
	config.camplitude = camplitude;
	config.mamplitude = mamplitude;
	config.cfrequency = cfrequency;
	config.mfrequency = mfrequency;

	return config;
}

ma_result cmads_modwave_init(cmads_modwave_config* pConfig, cmads_modwave* pModWave) {

	ma_result result;
	ma_data_source_config baseConfig;

	baseConfig = ma_data_source_config_init();
	baseConfig.vtable = &g_cmads_modwave_vtable;

	result = ma_data_source_init(&baseConfig, &pModWave->base);
	if (result != MA_SUCCESS)
		return result;

	if (pModWave == NULL)
		return MA_INVALID_ARGS;

	MA_ZERO_OBJECT(pModWave);

	pModWave->config = *pConfig;
	pModWave->time = 0;

	return MA_SUCCESS;
}

void cmads_modwave_uninit(cmads_modwave* pModWave) {

	if (pModWave == NULL)
		return;

	ma_data_source_uninit(&pModWave->base);
}
