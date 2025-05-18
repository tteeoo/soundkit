#include "adsrgen.cmdl.h"
#include "sk_adsr.h"
#include "generic_process.h"

#include <unistd.h>
#include <stdio.h>

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

ma_result process_function(void* vADSR, void* out, const void* in, ma_uint32 count) {
	return sk_adsr_process_pcm_frames((sk_adsr*)vADSR, out, in, count);
}

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}
	sk_adsr_config adsrConfig = sk_adsr_config_init(CHANNELS, SAMPLE_RATE,
		ai.attack_arg, ai.decay_arg, ai.sustain_arg, ai.coeff_arg, ai.release_arg, (ma_bool8)ai.exponential_flag);

	sk_adsr adsr;
	sk_adsr_init(&adsrConfig, &adsr);

	int fds[2];
	pipe(fds);
	dup2(fds[0], 0);
	if (fork() == 0) {
		close(fds[0]);
		float one = 1;
		while (1) {
			for (int i = 0; i < BATCH_SIZE*CHANNELS; i++)
				if (write(fds[1], &one, sizeof(float)) == -1)
					break;
			sleep(BATCH_SIZE / SAMPLE_RATE);
		}
	}
	close(fds[1]);
	read(0, &one, sizeof(float));

	forward_data((void *)&adsr, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	(void)argc;
	return 0;
}
