#ifndef generic_module_h
#define generic_module_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

ma_result playback_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate);
ma_result forward_data(ma_data_source* pData, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size);

#endif
