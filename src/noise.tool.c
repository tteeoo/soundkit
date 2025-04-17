#include <unistd.h>

#include "noise.cmdl.h"
#include "generic_source.h"

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
#define BATCH_SIZE   100

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	ma_noise_type type;
	switch (ai.type_arg) {
		case type_arg_w:
			type = ma_noise_type_white;
			break;
		case type_arg_p:
			type = ma_noise_type_pink;
			break;
		case type_arg_b:
			type = ma_noise_type_brownian;
			break;
		case type__NULL:
			exit(1);
	}

	ma_noise noise;
	ma_noise_config noiseConfig;

	noiseConfig = ma_noise_config_init(FORMAT, CHANNELS, type, getpid(), ai.amplitude_arg);
	ma_noise_init(&noiseConfig, NULL, &noise);

	if (isatty(1))
		playback_data((ma_data_source*)&noise, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&noise, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	ma_noise_uninit(&noise, NULL);
   
	(void)argc;
	return 0;
}
