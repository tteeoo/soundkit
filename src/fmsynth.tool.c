#include "generic_module.h"
#include "cmads_modwave.h"

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

int main(int argc, char** argv) {

	cmads_modwave modWave;
	cmads_modwave_config modWaveConfig;

	// TODO: input validation, custom waveforms
	modWaveConfig = cmads_modwave_config_init(FORMAT, CHANNELS, SAMPLE_RATE, ma_waveform_type_sine,
			atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]));
	cmads_modwave_init(&modWaveConfig, &modWave);

	if (isatty(1))
		playback_data((ma_data_source*)&modWave, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&modWave, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
    
	cmads_modwave_uninit(&modWave);
   
	(void)argc;
	return 0;
}
