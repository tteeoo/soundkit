#ifndef sk_adsr_h
#define sk_adsr_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

typedef enum {
	attack,
	decay,
	sustain,
	release
} adsr_state;

//
// struct definitions
//
typedef struct {
	ma_uint32 channels;
	ma_uint32 sample_rate;
	float attack_time;
	float decay_time;
	float sustain_time;
	float sustain_coeff;
	float release_time;
	ma_bool8 exponential;
} sk_adsr_config;

typedef struct {
	sk_adsr_config config;
	adsr_state state;
	ma_uint32 frames;
} sk_adsr;


// struct init/uninit methods
sk_adsr_config sk_adsr_config_init(ma_uint32 channels, ma_uint32 sample_rate, float attack_time, float decay_time, float sustain_time, float sustain_multiplier, float release_time, ma_bool8 exponential);
ma_result sk_adsr_init(sk_adsr_config* pConfig, sk_adsr* adsr);
ma_result sk_adsr_process_pcm_frames(sk_adsr* pADSR, void* out, const void* in, ma_uint32 count);

#endif
