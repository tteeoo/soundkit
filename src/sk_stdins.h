#ifndef sk_stdins_h
#define sk_stdins_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// struct definitions
//
typedef struct {
	ma_format format;
	ma_uint32 channels;
	ma_uint32 sampleRate;
} sk_stdins_config;

typedef struct {
	ma_data_source_base base;
	sk_stdins_config config;
} sk_stdins;

// struct init/uninit methods
sk_stdins_config sk_stdins_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate);
ma_result sk_stdins_init(sk_stdins_config* pConfig, sk_stdins* pStdins);
void sk_stdins_uninit(sk_stdins* pStdins);

#endif
