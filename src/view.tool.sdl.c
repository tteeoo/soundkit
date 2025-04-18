#include <SDL.h>

#include "generic_process.h"

#define MA_NO_GENERATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define WIDTH        800
#define HEIGHT       400
#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   4000

ma_result process_function(void* vRenderer, void* out, const void* in, ma_uint32 count) {

	SDL_Renderer* renderer = (SDL_Renderer*)vRenderer;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return -1;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);


	// Currently only defaults to the first channel
	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < count - 1; iFrame++) {
		int x1 = (iFrame * WIDTH) / count;
		int x2 = ((iFrame+1) * WIDTH) / count;
		int y1 = HEIGHT/2 + (inFloat[iFrame*CHANNELS] * HEIGHT/2);
		int y2 = HEIGHT/2 + (inFloat[(iFrame+1)*CHANNELS] * HEIGHT/2);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
			outFloat[iFrame*CHANNELS + iChannel] = inFloat[iFrame*CHANNELS + iChannel];
	}
	for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
		outFloat[(count-1)*CHANNELS + iChannel] = inFloat[(count-1)*CHANNELS + iChannel];

	SDL_RenderPresent(renderer);

	return MA_SUCCESS;

	(void)out;
}

int main(int argc, char** argv) {

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("soundkit view", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	forward_data((void*)renderer, CHANNELS, SAMPLE_RATE, BATCH_SIZE);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    
	(void)argc;
	(void)argv;
	return 0;
}
