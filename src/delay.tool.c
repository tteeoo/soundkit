#include "delay.cmdl.h"
#include "generic_process.h"

#define MA_NO_GENERATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   1000

ma_result process_function(void* delay, void* out, const void* in, ma_uint32 count) {
	return ma_delay_process_pcm_frames((ma_delay*)delay, out, in, count);
}

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	ma_delay_config delayConfig = ma_delay_config_init(CHANNELS, SAMPLE_RATE, (ma_uint32)(SAMPLE_RATE * ai.time_arg), ai.decay_arg);

	ma_delay delay;
	ma_delay_init(&delayConfig, NULL, &delay);

	forward_data((void*)&delay, CHANNELS, SAMPLE_RATE, BATCH_SIZE);

	ma_delay_uninit(&delay, NULL);
    
	(void)argc;
	return 0;
}
