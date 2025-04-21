#include "envelope.cmdl.h"
#include "sk_adsr.h"
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
#define BATCH_SIZE   100

ma_result process_function(void* vADSR, void* out, const void* in, ma_uint32 count) {
	return sk_adsr_process_pcm_frames((sk_adsr*)vADSR, out, in, count);
}

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	sk_adsr_config adsrConfig = sk_adsr_config_init(CHANNELS, SAMPLE_RATE,
		ai.attack_arg, ai.decay_arg, ai.sustain_arg, ai.coeff_arg, ai.release_arg);

	sk_adsr adsr;
	sk_adsr_init(&adsrConfig, &adsr);

	forward_data((void *)&adsr, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	(void)argc;
	return 0;
}
