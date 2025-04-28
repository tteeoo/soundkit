#ifndef sk_modwave_h
#define sk_modwave_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// struct definitions
//
typedef struct {
	ma_uint32 channels;
	ma_uint32 sample_rate;
	ma_waveform_type type;
	double camplitude;
	double mamplitude;
	double cfrequency;
	double mfrequency;
	ma_data_source* mod_source;
} sk_modwave_config;

typedef struct {
	ma_data_source_base base;
	sk_modwave_config config;
	double time;
	float* phaseAccum;
} sk_modwave;

// Direct modwave functions
void sk_modwave_read_pcm_frames(sk_modwave* pModWave, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
void sk_modwave_seek_to_pcm_frame(sk_modwave* pModWave, ma_uint64 frameIndex);

// struct init/uninit methods
sk_modwave_config sk_modwave_config_init(ma_uint32 channels, ma_uint32 sample_rate, ma_waveform_type type, double camplitude, double mamplitude, double cfrequency, double mfrequency, ma_data_source* mod_source);
ma_result sk_modwave_init(sk_modwave_config* pConfig, sk_modwave* pModWave);
void sk_modwave_uninit(sk_modwave* pModWave);

#endif
