#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include "sk_stdins.h"
#include "generic_process.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

void precise_sleep(double seconds) {
	struct timespec req;
	req.tv_sec = (time_t)seconds;
	req.tv_nsec = (long)((seconds - (time_t)seconds) * 1e9);
	nanosleep(&req, NULL);
}

ma_result forward_data(void* process_struct, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size) {

	setvbuf(stdout, NULL, _IONBF, 0);

	sk_stdins_config stdinsConfig = sk_stdins_config_init(ma_format_f32, channels, sample_rate);

	sk_stdins stdins;
	sk_stdins_init(&stdinsConfig, &stdins);

	float in[channels * batch_size];
	float out[channels * batch_size];
	for (ma_uint32 i = 0; i < channels * batch_size; i++) {
		in[i] = 0;
		out[i] = 0;
	}
	while (1) {

		ma_result result = ma_data_source_read_pcm_frames((ma_data_source*)&stdins, in, batch_size, NULL);
		if (result != MA_SUCCESS)
			break;

		result = process_function(process_struct, &out, &in, batch_size);
		if (result != MA_SUCCESS)
			break;

		if (write(1, &out, batch_size * sizeof(float) * channels) == -1)
			break;
		fsync(1);

		precise_sleep(0.5 * (double)batch_size / (double)sample_rate);
	}

	sk_stdins_uninit(&stdins);

	return MA_SUCCESS;
}
