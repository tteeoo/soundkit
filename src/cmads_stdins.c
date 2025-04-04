#include <unistd.h>

#include "cmads_stdins.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// vtable bindings
//
static ma_result cmads_stdins_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {

	//MA_ASSERT(pDataSource != NULL);
	//MA_ASSERT(pFramesOut != NULL);

	cmads_stdins* pStdins = (cmads_stdins*)pDataSource;

	float s;
	if (pStdins->config.format == ma_format_f32) {
		float* pFramesOutF32 = (float*)pFramesOut;
		for (ma_uint64 iFrame = 0; iFrame < frameCount; iFrame += 1) {

			for (ma_uint64 iChannel = 0; iChannel < pStdins->config.channels; iChannel += 1) {
				read(0, &s, sizeof(float));
				pFramesOutF32[iFrame*pStdins->config.channels + iChannel] = s;
			}
		}
	}

	if (pFramesRead != NULL)
		*pFramesRead = frameCount;

	return MA_SUCCESS;
}

static ma_result cmads_stdins_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {

	cmads_stdins* pStdins = (cmads_stdins*)pDataSource;

	*pFormat = pStdins->config.format;
	*pChannels = pStdins->config.channels;
	*pSampleRate = pStdins->config.sampleRate;
	ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pStdins->config.channels);

	return MA_SUCCESS;
}

static ma_data_source_vtable g_cmads_stdins_vtable = {
	cmads_stdins_on_read,
	NULL, // No seek
	cmads_stdins_on_get_data_format,
	NULL, // No cursor
	NULL, // No get_length
	NULL, // No set_looping
	0
};

//
// struct init/uninit methods
//
cmads_stdins_config cmads_stdins_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate) {

	cmads_stdins_config config;
	//MA_ZERO_OBJECT(&config);

	config.format = format;
	config.channels = channels;
	config.sampleRate = sampleRate;

	return config;
}

ma_result cmads_stdins_init(cmads_stdins_config* pConfig, cmads_stdins* pStdins) {

	ma_result result;
	ma_data_source_config baseConfig;

	baseConfig = ma_data_source_config_init();
	baseConfig.vtable = &g_cmads_stdins_vtable;

	//MA_ZERO_OBJECT(pStdins);

	result = ma_data_source_init(&baseConfig, &pStdins->base);
	if (result != MA_SUCCESS)
		return result;

	if (pStdins == NULL)
		return MA_INVALID_ARGS;

	pStdins->config = *pConfig;

	return MA_SUCCESS;
}

void cmads_stdins_uninit(cmads_stdins* pStdins) {

	if (pStdins == NULL)
		return;

	ma_data_source_uninit(&pStdins->base);
}
