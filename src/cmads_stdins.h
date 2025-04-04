#ifndef cmads_stdins_h
#define cmads_stdins_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// struct definitions
//
typedef struct {
	ma_format format;
	ma_uint32 channels;
	ma_uint32 sampleRate;
} cmads_stdins_config;

typedef struct {
	ma_data_source_base base;
	cmads_stdins_config config;
} cmads_stdins;

// struct init/uninit methods
cmads_stdins_config cmads_stdins_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate);
ma_result cmads_stdins_init(cmads_stdins_config* pConfig, cmads_stdins* pStdins);
void cmads_stdins_uninit(cmads_stdins* pStdins);

#endif
