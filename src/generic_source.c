#include <unistd.h>
#include <stdio.h>

#include "sk_stdins.h"
#include "generic_source.h"

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	ma_data_source_read_pcm_frames(pDevice->pUserData, pOutput, frameCount, NULL);

	(void)pInput;
}

ma_result playback_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate) {

	ma_device device;
	ma_device_config deviceConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = ma_format_f32;
	deviceConfig.playback.channels = channels;
	deviceConfig.sampleRate        = sample_rate;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = pData;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		return -4;
	}

	if (isatty(1))
		printf("Device Name: %s\n", device.playback.name);

	if (ma_device_start(&device) != MA_SUCCESS) {
		if (isatty(1)) 
			printf("Failed to start playback device.\n");
		ma_device_uninit(&device);
		return -5;
	}

	if (isatty(1))
		printf("Press key to quit...\n");
	getchar();

	ma_device_uninit(&device);

	return MA_SUCCESS;
}

ma_result forward_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size) {

	setvbuf(stdout, NULL, _IONBF, 0);

	float s[channels * batch_size];
	while (1) {
		ma_result result = ma_data_source_read_pcm_frames(pData, s, batch_size, NULL);
		if (result != MA_SUCCESS)
			break;
		if (write(1, &s, batch_size * sizeof(float) * channels) == -1)
			break;
		fsync(1);
		sleep(batch_size / sample_rate);
	}

	return MA_SUCCESS;
}
