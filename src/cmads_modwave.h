#ifndef cmads_modwave_h
#define cmads_modwave_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// struct definitions
//
typedef struct {
	ma_format format;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	ma_waveform_type type; // TODO: use mtype and ctype for waves
	double camplitude;
	double mamplitude;
	double cfrequency;
	double mfrequency;
} cmads_modwave_config;

typedef struct {
	ma_data_source_base base;
	cmads_modwave_config config;
	double time;
} cmads_modwave;

// Direct modwave functions
void cmads_modwave_read_pcm_frames(cmads_modwave* pModWave, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
void cmads_modwave_seek_to_pcm_frame(cmads_modwave* pModWave, ma_uint64 frameIndex);

// struct init/uninit methods
cmads_modwave_config cmads_modwave_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_waveform_type type, double camplitude, double mamplitude, double cfrequency, double mfrequency);
ma_result cmads_modwave_init(cmads_modwave_config* pConfig, cmads_modwave* pModWave);
void cmads_modwave_uninit(cmads_modwave* pModWave);

#endif
