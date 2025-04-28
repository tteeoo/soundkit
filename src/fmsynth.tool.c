#include "fmsynth.cmdl.h"
#include "generic_source.h"
#include "sk_stdins.h"
#include "sk_modwave.h"

#include <stdio.h>

#define MA_NO_GENERATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   100

int main(int argc, char** argv) {

	sk_stdins stdins;
	sk_stdins_config stdinsConfig = sk_stdins_config_init(ma_format_f32, CHANNELS, SAMPLE_RATE);
	sk_modwave_config modwaveConfig;

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	ma_waveform_type type;
	switch (ai.type_arg) {
		case type_arg_s:
			type = ma_waveform_type_sine;
			break;
		case type_arg_u:
			type = ma_waveform_type_square;
			break;
		case type_arg_w:
			type = ma_waveform_type_sawtooth;
			break;
		case type_arg_t:
			type = ma_waveform_type_triangle;
			break;
		case type__NULL:
			exit(1);
	}

	if (!isatty(0)) {
		sk_stdins_init(&stdinsConfig, &stdins);
		modwaveConfig = sk_modwave_config_init(CHANNELS, SAMPLE_RATE, type,
			ai.amplitude_arg, ai.mamplitude_arg, ai.frequency_arg, ai.mfrequency_arg, (ma_data_source*)&stdins);
	} else {
		if (ai.mfrequency_arg == 0) {
			dprintf(2, "Without a modulation source piped in, a modulation frequency (-s) must be specified\n");
			exit(1);
		}
		modwaveConfig = sk_modwave_config_init(CHANNELS, SAMPLE_RATE, ma_waveform_type_sine,
			ai.amplitude_arg, ai.mamplitude_arg, ai.frequency_arg, ai.mfrequency_arg, NULL);
	}

	sk_modwave modwave;
	sk_modwave_init(&modwaveConfig, &modwave);

	if (isatty(1))
		playback_data((ma_data_source*)&modwave, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&modwave, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
  
	if (!isatty(0))
		sk_stdins_uninit(&stdins);
	sk_modwave_uninit(&modwave);
   
	(void)argc;
	return 0;
}
