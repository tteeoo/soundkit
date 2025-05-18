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

ma_result process_function(void* data, void* out, const void* in, ma_uint32 count) {

	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < count; iFrame++)
		for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
			outFloat[CHANNELS*iFrame + iChannel] = -inFloat[CHANNELS*iFrame + iChannel];

	(void)data;
	return MA_SUCCESS;
}

int main(int argc, char** argv) {

	forward_data((void*)NULL, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	(void)argc;
	(void)argv;
	return 0;
}
