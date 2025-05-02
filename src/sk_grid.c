#include <unistd.h>
#include <fcntl.h>

#include "sk_grid.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// vtable bindings
//
static ma_result sk_grid_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {

	//MA_ASSERT(pDataSource != NULL);
	//MA_ASSERT(pFramesOut != NULL);

	sk_grid* pGrid = (sk_grid*)pDataSource;

	float s;
	float* outFloat = (float*)pFramesOut;
	for (ma_uint64 iFrame = 0; iFrame < frameCount; iFrame += 1) {
		for (ma_uint64 iChannel = 0; iChannel < pGrid->config.channels; iChannel += 1) {
			for (int iPipe = 0; iPipe < pGrid->config.count; iPipe++) {
				/*if (read(pGrid->config.fds[iPipe][0], &s, sizeof(float)) == 0)*/
				/*	continue;*/
				read(pGrid->config.fds[iPipe][0], &s, sizeof(float));
				outFloat[iFrame*pGrid->config.channels + iChannel] += s;
			}
		}
	}

	if (pFramesRead != NULL)
		*pFramesRead = frameCount;

	return MA_SUCCESS;
}

static ma_result sk_grid_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {

	sk_grid* pgrid = (sk_grid*)pDataSource;

	*pFormat = pgrid->config.format;
	*pChannels = pgrid->config.channels;
	*pSampleRate = pgrid->config.sampleRate;
	ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pgrid->config.channels);

	return MA_SUCCESS;
}

static ma_data_source_vtable g_sk_grid_vtable = {
	sk_grid_on_read,
	NULL, // No seek
	sk_grid_on_get_data_format,
	NULL, // No cursor
	NULL, // No get_length
	NULL, // No set_looping
	0
};

//
// struct init/uninit methods
//
sk_grid_config sk_grid_config_init(ma_uint32 channels, ma_uint32 sampleRate, int (*fds)[2], int count) {

	sk_grid_config config;

	config.channels = channels;
	config.sampleRate = sampleRate;
	config.fds = fds;
	config.count = count;

	/*for (int i = 0; i < count; i++)*/
	/*	fcntl(fds[i][0], F_SETFL, fcntl(fds[i][0], F_GETFL) | O_NONBLOCK);*/

	return config;
}

ma_result sk_grid_init(sk_grid_config* pConfig, sk_grid* pGrid) {

	ma_result result;
	ma_data_source_config baseConfig;

	baseConfig = ma_data_source_config_init();
	baseConfig.vtable = &g_sk_grid_vtable;

	result = ma_data_source_init(&baseConfig, &pGrid->base);
	if (result != MA_SUCCESS)
		return result;

	if (pGrid == NULL)
		return MA_INVALID_ARGS;

	pGrid->config = *pConfig;

	return MA_SUCCESS;
}

void sk_grid_uninit(sk_grid* pGrid) {

	if (pGrid == NULL)
		return;

	ma_data_source_uninit(&pGrid->base);
}
