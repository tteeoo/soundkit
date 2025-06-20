#ifndef generic_process_h
#define generic_process_h

#undef MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

extern ma_result process_function(void* process_struct, void* out, const void* in, ma_uint32 count);
void precise_sleep(double seconds);
ma_result forward_data(void* process_struct, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 batch_size);

#endif
