#ifndef generic_source_h
#define generic_source_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

void precise_sleep(double seconds);
ma_result playback_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate);
ma_result forward_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size);

#endif
