#include "decode.cmdl.h"
#include "generic_source.h"

#define MA_NO_GENERATION
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

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	ma_decoder_config decoderConfig = ma_decoder_config_init(FORMAT, CHANNELS, SAMPLE_RATE);

	ma_decoder decoder;
	ma_decoder_init_file(ai.path_arg, &decoderConfig, &decoder);

	if (isatty(1))
		playback_data((ma_data_source*)&decoder, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&decoder, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	ma_decoder_uninit(&decoder);
   
	(void)argc;
	return 0;
}
