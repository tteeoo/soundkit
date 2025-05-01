#ifndef sk_grid_h
#define sk_grid_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

//
// struct definitions
//
typedef struct {
	ma_format format;
	ma_uint32 channels;
	ma_uint32 sampleRate;
	int (*fds)[2];
	int count;
} sk_grid_config;

typedef struct {
	ma_data_source_base base;
	sk_grid_config config;
} sk_grid;

// struct init/uninit methods
sk_grid_config sk_grid_config_init(ma_uint32 channels, ma_uint32 sampleRate, int (*fds)[2], int count);
ma_result sk_grid_init(sk_grid_config* pConfig, sk_grid* pGrid);
void sk_grid_uninit(sk_grid* pGrid);

#endif
