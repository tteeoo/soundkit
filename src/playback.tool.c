#include <stdio.h>
#include <unistd.h>

#include "sk_stdins.h"

#define MA_NO_GENERATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	MA_ASSERT(pDevice->playback.channels == CHANNELS);

	MA_ASSERT(pDevice->pUserData != NULL);

	ma_data_source_read_pcm_frames(pDevice->pUserData, pOutput, frameCount, NULL);
	
	if (!isatty(1))
		write(1, pOutput, frameCount * sizeof(float) * CHANNELS);

	(void)pInput;
}

int main(int argc, char** argv) {

	ma_device device;
	ma_device_config deviceConfig;
	sk_stdins stdins;
	sk_stdins_config stdinsConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = FORMAT;
	deviceConfig.playback.channels = CHANNELS;
	deviceConfig.sampleRate        = SAMPLE_RATE;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = &stdins;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		return -4;
	}

	if (isatty(1))
		printf("Device Name: %s\n", device.playback.name);

	stdinsConfig = sk_stdins_config_init(device.playback.format, device.playback.channels, device.sampleRate);
	sk_stdins_init(&stdinsConfig, &stdins);

	if (ma_device_start(&device) != MA_SUCCESS) {
		if (isatty(1)) 
			printf("Failed to start playback device.\n");
		ma_device_uninit(&device);
		return -5;
	}

	while(1);

	ma_device_uninit(&device);
	sk_stdins_uninit(&stdins);

	(void)argc;
	(void)argv;
	return 0;
}
