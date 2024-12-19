#include <stdio.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include "sample.h"

int main(int argc, char **argv) {

	// Initialize audio engine
	ma_result result;
	ma_engine engine;
	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		return result;
	}

	// End cleanly
	ma_engine_uninit(&ma_engine);
	return 0;
}
