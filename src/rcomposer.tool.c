#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "generic_source.h"
#include "sk_grid.h"

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
#define BATCH_SIZE   1000

void precise_sleep(double seconds) {
	struct timespec req;
	req.tv_sec = (time_t)seconds;
	req.tv_nsec = (long)((seconds - (time_t)seconds) * 1e9);
	nanosleep(&req, NULL);
}

int main(int argc, char** argv) {

	if (argc != 2) {
		fprintf(stderr, "Usage: rcomposer <rhythm file>\n");
		exit(1);
	}

	FILE* rfp = fopen(argv[1], "r");
	if (!rfp) {
		perror("Error opening rhythm file");
		exit(1);
	}

	float cpm = -1;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	int gathering = 1;
	int si = 0;
	char symbols[1000];
	char* signals[1000];
	char** rhythm;
	int rlen = -1;
	int pipes[1000][2];

	while ((read = getline(&line, &len, rfp)) != -1) {
		if (line[0] == '#' || line[0] == ' ' || line[0] == '\n' || len == 1)
			continue;
		if (line[0] == '|')
			gathering = 0;
		if (cpm == -1) {
			unsigned int bpm;
			char chars[100];
			sscanf(line, "%u%s", &bpm, chars);
			int nc = 0;
			for (int i = 0; chars[i] != '\0'; i++)
				if (chars[i] == '-')
					nc++;
			if (nc == 0)
				nc = 1;
			cpm = (float)bpm * (float)nc;
		} else if (gathering) {
			signals[si] = (char*)malloc(sizeof(char) * (len-1));
			symbols[si] = line[0];
			strcat(signals[si], &line[2]);
			signals[si][strlen(signals[si])-1] = '\0';
			fprintf(stderr, "%c %s\n", symbols[si], signals[si]);
			pipe(pipes[si]);
			si++;
		} else {
			if (rlen == -1) {
				rlen = strchr(&line[1], '|') - line - 1;
				rhythm = (char**)malloc(sizeof(char*) * rlen);
				for (int i = 0; i < rlen; i++) {
					rhythm[i] = (char*)malloc(sizeof(char) * 1000);
				}
			} else {
				if (strchr(&line[1], '|') - line - 1 != rlen) {
					fprintf(stderr, "Rhythm length mismatch\n");
					exit(1);
				}
			}
			for (size_t i = 1; i < len; i++) {
				if (line[i] == '|')
					break;
				char temp[2] = {line[i], '\0'};
				strcat(rhythm[i-1], temp);
			}
		}
	}
	fclose(rfp);
	fprintf(stderr, "timing:%f instruments:%d\n", 60 / cpm, si);

	// TODO
	// - synchronization
	// - signal handling to kill all processes after ^C
	// - have as many proc storage for each signal as rlen, to allow overlap
	// 	(but then sk_grid must have multiple inputs for each instrument)
	if (fork() == 0) {

		int procs[1000] = {0};
		while (true) {

			for (int i = 0; i < rlen; i++) {
				for (int sj = 0; sj < si; sj++) {

					if (strchr(rhythm[i], symbols[sj])) {
						if (procs[sj] != 0) {
							int e = killpg(procs[sj], 9);
							if (e != 0)
								perror("killing");
						}

						if ((procs[sj] = fork()) == 0) {
							/*close(pipes[i][0]);*/
							setpgid(0, 0);
							dup2(pipes[sj][1], 1);
							execlp("/bin/sh", "sh", "-c", signals[sj], NULL);
						}
					}
				}

				precise_sleep(60 / cpm);
			}
		}
	}

	sk_grid_config gridConfig = sk_grid_config_init(CHANNELS, SAMPLE_RATE, pipes, si);

	sk_grid grid;
	sk_grid_init(&gridConfig, &grid);

	if (isatty(1))
		playback_data((ma_data_source*)&grid, CHANNELS, SAMPLE_RATE);
	else
		forward_data((ma_data_source*)&grid, CHANNELS, SAMPLE_RATE, BATCH_SIZE);

	sk_grid_uninit(&grid);

	return 0;
}
