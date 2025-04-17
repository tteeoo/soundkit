#include <stdio.h>
#include <unistd.h>

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

ma_result process_function(void* hpf,void* out,const void* in,ma_uint32 count) {
	return ma_hpf_process_pcm_frames((ma_hpf*)hpf, out, in, count);
}

int main(int argc, char** argv) {

	ma_hpf_config hpfConfig = ma_hpf_config_init(FORMAT, CHANNELS, SAMPLE_RATE, SAMPLE_RATE / atof(argv[1]), atoi(argv[2]));

	ma_hpf hpf;
	ma_hpf_init(&hpfConfig, NULL, &hpf);

	forward_data((void*)&hpf, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	ma_hpf_uninit(&hpf, NULL);

	(void)argc;
	return 0;
}
