#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <SDL.h>

#include "cmads_stdins.h"
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#define WIDTH        800
#define HEIGHT       400
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   4000

ma_result forward_data(cmads_stdins stdins, SDL_Renderer* renderer) {

	float s[CHANNELS * BATCH_SIZE];
	int running = 1;
	while (running) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
		    if (event.type == SDL_QUIT) running = 0;
		}

		ma_data_source_read_pcm_frames((ma_data_source*)&stdins, s, BATCH_SIZE, NULL);
		write(1, &s, BATCH_SIZE * sizeof(float) * CHANNELS);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Currently only defaults to the first channel
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		for (int i = 0; i < BATCH_SIZE - 1; i++) {
			int x1 = (i * WIDTH) / BATCH_SIZE;
			int x2 = ((i+1) * WIDTH) / BATCH_SIZE;
			int y1 = HEIGHT/2 + (s[i*CHANNELS] * HEIGHT/2);
			int y2 = HEIGHT/2 + (s[(i+1)*CHANNELS] * HEIGHT/2);
			SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		}

		SDL_RenderPresent(renderer);
		sleep(BATCH_SIZE / SAMPLE_RATE);
	}

	return MA_SUCCESS;
}

int main(int argc, char** argv) {

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("soundkit view", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	cmads_stdins stdins;
	cmads_stdins_config stdinsConfig;

	stdinsConfig = cmads_stdins_config_init(FORMAT, CHANNELS, SAMPLE_RATE);
	cmads_stdins_init(&stdinsConfig, &stdins);

	forward_data(stdins, renderer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    
	cmads_stdins_uninit(&stdins);

	(void)argc;
	(void)argv;
	return 0;
}
