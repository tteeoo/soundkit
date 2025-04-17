#include <unistd.h>
#include <stdio.h>

#include "cmads_stdins.h"
#include "generic_process.h"
#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

ma_result forward_data(void* process_struct, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size) {

	cmads_stdins_config stdinsConfig = cmads_stdins_config_init(ma_format_f32, channels, sample_rate);

	cmads_stdins stdins;
	cmads_stdins_init(&stdinsConfig, &stdins);

	float s[channels * batch_size];
	while (1) {

		ma_result result = ma_data_source_read_pcm_frames((ma_data_source*)&stdins, s, batch_size, NULL);
		if (result != MA_SUCCESS)
			break;

		result = process_function(process_struct, &s, &s, batch_size);
		if (result != MA_SUCCESS)
			break;

		write(1, &s, batch_size * sizeof(float) * channels);

		sleep(batch_size / sample_rate);
	}

	cmads_stdins_uninit(&stdins);

	return MA_SUCCESS;
}
