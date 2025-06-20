#include <pthread.h>
#include <unistd.h>
#include <time.h>
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
#define BUFFER_SIZE  20000
#define FORMAT       ma_format_f32
#define CHANNELS     2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   4000

ma_pcm_rb ring_buffer;

void precise_sleep(double seconds) {
	struct timespec req;
	req.tv_sec = (time_t)seconds;
	req.tv_nsec = (long)((seconds - (time_t)seconds) * 1e9);
	nanosleep(&req, NULL);
}

void render(SDL_Renderer* renderer) {

	ma_uint32 frameCount;
	void* readBuffer;
	float rbFloat[(BATCH_SIZE) * CHANNELS];

	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				return;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

		frameCount = BATCH_SIZE;
		ma_pcm_rb_acquire_read(&ring_buffer, &frameCount, &readBuffer);
		MA_COPY_MEMORY(rbFloat, readBuffer, frameCount * sizeof(float) * CHANNELS);
		ma_pcm_rb_commit_read(&ring_buffer, frameCount);

		if (frameCount != 0) {
			float* rbFloat = (float*)readBuffer;
			// Currently only defaults to the first channel
			for (ma_uint32 iFrame = 0; iFrame < frameCount - 1; iFrame++) {
				int x1 = (iFrame * WIDTH) / frameCount;
				int x2 = ((iFrame+1) * WIDTH) / frameCount;
				int y1 = HEIGHT/2 - (rbFloat[iFrame*CHANNELS] * HEIGHT/2);
				int y2 = HEIGHT/2 - (rbFloat[(iFrame+1)*CHANNELS] * HEIGHT/2);
				SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
			}
			SDL_RenderPresent(renderer);
		}

		precise_sleep((double)BATCH_SIZE / (double)SAMPLE_RATE);
	}

	return;
}

ma_result process_function(void* nothing, void* out, const void* in, ma_uint32 frameCount) {

	void* writeBuffer;
	float* inFloat = (float*)in;
	float* outFloat = (float*)out;
	for (ma_uint32 iFrame = 0; iFrame < frameCount; iFrame++)
		for (ma_uint32 iChannel = 0; iChannel < CHANNELS; iChannel++)
			outFloat[iFrame*CHANNELS + iChannel] = inFloat[iFrame*CHANNELS + iChannel];

	ma_pcm_rb_acquire_write(&ring_buffer, &frameCount, &writeBuffer);
        MA_COPY_MEMORY(writeBuffer, in, frameCount * sizeof(float) * CHANNELS);
        ma_pcm_rb_commit_write(&ring_buffer, frameCount);

	return MA_SUCCESS;
	(void)nothing;
}

void* forward_thread(void* nothing) {
	forward_data(NULL, CHANNELS, SAMPLE_RATE, BATCH_SIZE);
	return NULL;
	(void)nothing;
}

int main(int argc, char** argv) {

	ma_pcm_rb_init(ma_format_f32, CHANNELS, BUFFER_SIZE, NULL, NULL, &ring_buffer);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("soundkit view", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	pthread_t pt;
	pthread_create(&pt, NULL, forward_thread, NULL);

	render(renderer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    
	(void)argc;
	(void)argv;
	return 0;
}
