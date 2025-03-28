#include <stdio.h>
#include <unistd.h>

#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#include "cmads_stdins.c"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

	MA_ASSERT(pDevice->pUserData != NULL);

	ma_data_source_read_pcm_frames(pDevice->pUserData, pOutput, frameCount, NULL);
}

int main(int argc, char** argv) {

	ma_device device;
	ma_device_config deviceConfig;
	cmads_stdins stdins;
	cmads_stdins_config stdinsConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = DEVICE_FORMAT;
	deviceConfig.playback.channels = DEVICE_CHANNELS;
	deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = &stdins;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		return -4;
	}

	if (isatty(1))
		printf("Device Name: %s\n", device.playback.name);

	stdinsConfig = cmads_stdins_config_init(device.playback.format, device.playback.channels, device.sampleRate);
	cmads_stdins_init(&stdinsConfig, &stdins);

	if (ma_device_start(&device) != MA_SUCCESS) {
		if (isatty(1)) 
			printf("Failed to start playback device.\n");
		ma_device_uninit(&device);
		return -5;
	}

	while(1);

	ma_device_uninit(&device);
	cmads_stdins_uninit(&stdins);
    
	return 0;
}
