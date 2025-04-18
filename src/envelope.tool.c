#include "envelope.cmdl.h"
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

typedef struct {
	float attack_time;
	ma_bool8 attacking;
} sk_envelope;

static ma_result sk_envelope_init(sk_envelope* envelope, float attack_time) {
	envelope->attack_time = attack_time;
	envelope->attacking = 1;
	return MA_SUCCESS;
}

ma_result process_function(void* vEnvelope, void* out, const void* in, ma_uint32 count) {

	static ma_uint32 i = 0;
	float time = (float)(i * BATCH_SIZE) / (float)SAMPLE_RATE;

	sk_envelope* envelope = (sk_envelope*) vEnvelope;
	if (envelope->attack_time <= time)
		envelope->attacking = 0;

	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < count; iFrame++)
		for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
			if (envelope->attacking)
				outFloat[iFrame*CHANNELS + iChannel] = inFloat[iFrame*CHANNELS + iChannel] * (time / envelope->attack_time );
			else
				outFloat[iFrame*CHANNELS + iChannel] = inFloat[iFrame*CHANNELS + iChannel];
		

	i++;

	return MA_SUCCESS;
}

int main(int argc, char** argv) {

	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}

	sk_envelope envelope;
	sk_envelope_init(&envelope, ai.attack_arg);

	forward_data((void *)&envelope, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	(void)argc;
	return 0;
}
