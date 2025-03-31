#include <stdio.h>
#include <unistd.h>

#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#include "cmads_modwave.c"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   100

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	MA_ASSERT(pDevice->playback.channels == CHANNELS);

	cmads_modwave* pModWave = (cmads_modwave*)pDevice->pUserData;
	MA_ASSERT(pModWave != NULL);

	cmads_modwave_read_pcm_frames(pModWave, pOutput, frameCount, NULL);
}

ma_result playback_data(cmads_modwave modWave) {

	ma_device device;
	ma_device_config deviceConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = FORMAT;
	deviceConfig.playback.channels = CHANNELS;
	deviceConfig.sampleRate        = SAMPLE_RATE;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = &modWave;

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

ma_result forward_data(cmads_modwave modWave) {

	float s[CHANNELS * BATCH_SIZE];
	while (1) {
		cmads_modwave_read_pcm_frames(&modWave, s, BATCH_SIZE, NULL);
		write(1, &s, BATCH_SIZE * sizeof(float) * CHANNELS);
		sleep(BATCH_SIZE / SAMPLE_RATE);
	}

	return MA_SUCCESS;
}

int main(int argc, char** argv) {

	cmads_modwave modWave;
	cmads_modwave_config modWaveConfig;

	// TODO: input validation
	modWaveConfig = cmads_modwave_config_init(FORMAT, CHANNELS, SAMPLE_RATE, ma_waveform_type_sine,
			atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]));
	cmads_modwave_init(&modWaveConfig, &modWave);

	if (isatty(1))
		playback_data(modWave);
	else
		forward_data(modWave);
    
	cmads_modwave_uninit(&modWave);
    
	return 0;
}
