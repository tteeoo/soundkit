#include "wave.cmdl.h"
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

	ma_waveform_type type;
	switch (ai.type_arg) {
		case type_arg_sn:
			type = ma_waveform_type_sine;
			break;
		case type_arg_sq:
			type = ma_waveform_type_square;
			break;
		case type_arg_sw:
			type = ma_waveform_type_sawtooth;
			break;
		case type__NULL:
			exit(1);
	}

	ma_waveform waveform;
	ma_waveform_config waveformConfig;

	waveformConfig = ma_waveform_config_init(FORMAT, CHANNELS, SAMPLE_RATE, type, ai.amplitude_arg, ai.frequency_arg);
	ma_waveform_init(&waveformConfig, &waveform);

	if (isatty(1))
		playback_data((ma_data_source*)&waveform, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&waveform, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	ma_waveform_uninit(&waveform);
   
	(void)argc;
	return 0;
}
