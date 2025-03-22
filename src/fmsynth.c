#include <stdio.h>
#include <unistd.h>

#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#include "cmads_modwave.c"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

	cmads_modwave* pModWave = (cmads_modwave*)pDevice->pUserData;
	MA_ASSERT(pModWave != NULL);

	cmads_modwave_read_pcm_frames(pModWave, pOutput, frameCount, NULL);
}

int main(int argc, char** argv) {

	// TODO manage sound playback based on output streams and options

	ma_device device;
	ma_device_config deviceConfig;
	cmads_modwave modWave;
	cmads_modwave_config modWaveConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = DEVICE_FORMAT;
	deviceConfig.playback.channels = DEVICE_CHANNELS;
	deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = &modWave;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Failed to open playback device.\n");
		return -4;
	}

	if (isatty(1))
		printf("Device Name: %s\n", device.playback.name);

	// TODO: input validation
	modWaveConfig = cmads_modwave_config_init(device.playback.format, device.playback.channels, device.sampleRate, ma_waveform_type_sine,
			atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]));
	cmads_modwave_init(&modWaveConfig, &modWave);

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
	cmads_modwave_uninit(&modWave);
    
	return 0;
}
