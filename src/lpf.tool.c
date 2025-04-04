#include <stdio.h>
#include <unistd.h>

#include "cmads_stdins.h"
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   100

ma_result forward_data(cmads_stdins stdins, ma_lpf lpf) {

	float s[CHANNELS * BATCH_SIZE];
	while (1) {
		ma_data_source_read_pcm_frames((ma_data_source*)&stdins, s, BATCH_SIZE, NULL);
		ma_lpf_process_pcm_frames(&lpf, &s, &s, BATCH_SIZE);
		write(1, &s, BATCH_SIZE * sizeof(float) * CHANNELS);
		sleep(BATCH_SIZE / SAMPLE_RATE);
	}

	return MA_SUCCESS;
}

int main(int argc, char** argv) {

	ma_lpf lpf;
	ma_lpf_config lpfConfig;
	cmads_stdins stdins;
	cmads_stdins_config stdinsConfig;

	lpfConfig = ma_lpf_config_init(FORMAT, CHANNELS, SAMPLE_RATE, SAMPLE_RATE / atof(argv[1]), atoi(argv[2]));
	ma_lpf_init(&lpfConfig, NULL, &lpf);
	stdinsConfig = cmads_stdins_config_init(FORMAT, CHANNELS, SAMPLE_RATE);
	cmads_stdins_init(&stdinsConfig, &stdins);

	forward_data(stdins, lpf);
    
	cmads_stdins_uninit(&stdins);

	(void)argc;
	return 0;
}
