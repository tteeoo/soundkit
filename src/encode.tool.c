#include <stdio.h>
#include <signal.h>

#include "encode.cmdl.h"
#include "generic_process.h"

#define MA_NO_GENERATION
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   1000

ma_result process_function(void* encoder, void* out, const void* in, ma_uint32 count) {
	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < count; iFrame++)
		for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
			outFloat[iFrame*CHANNELS + iChannel] = inFloat[iFrame*CHANNELS + iChannel];
	ma_uint64 written;
	ma_encoder_write_pcm_frames((ma_encoder*)encoder, in, count, &written);
	return MA_SUCCESS;
}

ma_encoder encoder;

void intHandler(int sig) {
	ma_encoder_uninit(&encoder);
	(void)sig;
}

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	signal(SIGINT, intHandler);

	ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, FORMAT, CHANNELS, SAMPLE_RATE);

	ma_encoder_init_file(ai.path_arg, &encoderConfig, &encoder);

	forward_data((void*)&encoder, CHANNELS, SAMPLE_RATE, BATCH_SIZE);

	ma_encoder_uninit(&encoder);
   
	(void)argc;
	return 0;
}
