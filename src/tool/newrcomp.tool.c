// TODO:
// drops in audio playback (not mixed frames)

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <time.h>

#include <sys/uio.h>

#include "newrcomp.cmdl.h"

#define MA_NO_GENERATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_ENGINE
#define MA_NO_NODE_GRAPH
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "../../miniaudio/miniaudio.h"

#define FORMAT	   float
#define CHANNELS	 2
#define SAMPLE_RATE  48000
#define BATCH_SIZE   1000

#define MAX_SOUNDS 256
#define MAX_CHAIN_LEN 32

int verbose = 0;

typedef struct {
	char *string;
	pid_t pids[32];
	int fds[2];
	int active;
} signal_chain;

typedef struct {
	signal_chain sounds[MAX_SOUNDS];
	struct pollfd pfds[MAX_SOUNDS];
	int active_count;

	pthread_mutex_t mutex;
	pthread_cond_t new_sound_cond;

	float temp_buffer[BATCH_SIZE * CHANNELS];
	float mix_buffer[BATCH_SIZE * CHANNELS];

	atomic_bool running;

	ma_rb* output_rb;
} threaded_mixer;

typedef struct {
	char* file_path;
	char symbols[1000];
	char* signals[1000];
	char** rhythm;
	float cpm;
	int rlen;
	int si;
} track;

typedef struct {
	track track_info;
	threaded_mixer* pMixer;
	atomic_bool sequencer_running;
} rcomposer;

void precise_sleep(double seconds) {
	struct timespec req;
	req.tv_sec = (time_t)seconds;
	req.tv_nsec = (long)((seconds - (time_t)seconds) * 1e9);
	nanosleep(&req, NULL);
}

int execute_signal_chain(signal_chain* pSound) {
	char* chain_copy = strdup(pSound->string);
	char* saveptr = NULL;
	int prev_pipe_read = -1;
	int pid_count = 0;
	int pipe_created = 0;  // Track if we created a pipe for this stage
	
	// Split by '|'
	char *cmd = strtok_r(chain_copy, "|", &saveptr);
	while (cmd) {
		// Trim whitespace
		while (*cmd == ' ') cmd++;
		
		// Check if there's another command after this one
		char *next_cmd = strtok_r(NULL, "|", &saveptr);
		pipe_created = 0;
		
		int pipefd[2] = {-1, -1};
		
		// Create pipe if there's another command after this one
		if (next_cmd) {
			if (pipe(pipefd) == -1) {
				free(chain_copy);
				return 0;
			}
			pipe_created = 1;
		}
		
		pid_t pid = fork();
		if (pid == 0) {  // Child process
			// Set up input from previous stage
			if (prev_pipe_read != -1) {
				dup2(prev_pipe_read, STDIN_FILENO);
				close(prev_pipe_read);
			}
			
			// Set up output to next stage or final output
			if (pipe_created) {  // Not last command
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[0]);
				close(pipefd[1]);
			} else {  // Last command
				dup2(pSound->fds[1], STDOUT_FILENO);
			}
			
			// Also redirect stderr to avoid terminal noise
			int devnull = open("/dev/null", O_WRONLY);
			if (devnull != -1) {
				dup2(devnull, STDERR_FILENO);
				close(devnull);
			}
			
			// Parse command and arguments
			char *argv[20];
			int argc = 0;
			
			// Need to copy the command again for parsing
			char *cmd_copy = strdup(cmd);
			char *token = strtok(cmd_copy, " ");
			while (token && argc < 19) {
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			argv[argc] = NULL;
			
			// Execute
			execvp(argv[0], argv);
			
			// If we get here, exec failed
			perror("execvp failed");
			free(cmd_copy);
			_exit(EXIT_FAILURE);
		}
		
		// Parent process
		pSound->pids[pid_count++] = pid;
		
		// Clean up previous pipe read end
		if (prev_pipe_read != -1) {
			close(prev_pipe_read);
		}
		
		// Set up for next iteration
		if (pipe_created) {
			// Close write end in parent
			close(pipefd[1]);
			// Save read end for next command's input
			prev_pipe_read = pipefd[0];
		}
		
		// Move to next command
		cmd = next_cmd;
	}
	
	free(chain_copy);
	return 1;
}

// Efficient pipe reading using vmsplice for zero-copy
ssize_t read_pipe_zero_copy(int pipefd, float *buffer, size_t frames) {
	struct iovec iov = {
		.iov_base = buffer,
		.iov_len = frames * CHANNELS * sizeof(float)  // Stereo
	};
	
	// vmsplice transfers data from user memory to pipe (or vice versa)
	// without copying through kernel buffers
	ssize_t bytes = vmsplice(pipefd, &iov, 1, SPLICE_F_NONBLOCK);
	
	if (bytes > 0) {
		// Data was "spliced" directly into our buffer
		return bytes / (CHANNELS * sizeof(float));  // Return frame count
	}
	
	// Fallback to normal read
	return read(pipefd, buffer, frames * CHANNELS * sizeof(float)) / (CHANNELS * sizeof(float));
}


ssize_t read_pipe_audio(int pipefd, float *buffer, size_t max_frames) {
	// Try to read complete buffers to reduce syscalls
	size_t total_bytes = 0;
	size_t target_bytes = max_frames * CHANNELS * sizeof(float); 
	
	while (total_bytes < target_bytes) {
		ssize_t bytes = read(pipefd, (char*)buffer + total_bytes, target_bytes - total_bytes);
		
		if (bytes > 0) {
			total_bytes += bytes;
		} else if (bytes == 0) {
			// EOF
			break;
		} else if (errno == EAGAIN) {
			// No more data available now
			break;
		} else {
			// Error
			return -1;
		}
	}
	
	return total_bytes / (CHANNELS * sizeof(float));  // Frames read
}

// pipe_mixer.c
void stop_sound_pipe(threaded_mixer* mixer, int index) {

	pthread_mutex_lock(&mixer->mutex);

	if (index < 0 || index >= mixer->active_count) return;
	
	signal_chain* pSound = &mixer->sounds[index];
	
	// Kill all processes in the pipe chain
	for (int i = 0; (i < MAX_CHAIN_LEN) && (pSound->pids[i] > 0); i++) {
		if (kill(pSound->pids[i], SIGTERM) == 0) {
			// Give process a chance to exit cleanly
			int status;
			waitpid(pSound->pids[i], &status, WNOHANG);
			
			// Force kill if still running
			/*usleep(10000);  // 10ms grace period*/
			if (waitpid(pSound->pids[i], &status, WNOHANG) == 0) {
				kill(pSound->pids[i], SIGKILL);
				waitpid(pSound->pids[i], &status, 0);
			}
		}
	}

	// Close pipe file descriptor
	if (pSound->fds[0] != -1) {
		close(pSound->fds[0]);
		pSound->fds[0] = -1;
	}

	mixer->active_count--;
	int new_active = mixer->active_count;

	// Remove from poll array by swapping with last element
	mixer->sounds[index] = mixer->sounds[new_active];
	mixer->pfds[index] = mixer->pfds[new_active];

	// Clear the now-empty slot
	memset(&mixer->sounds[new_active], 0, sizeof(signal_chain));
	mixer->pfds[new_active].fd = -1;
	mixer->pfds[new_active].events = 0;
	mixer->pfds[new_active].revents = 0;

	pthread_mutex_unlock(&mixer->mutex);
}

void* mixer_thread(void* arg) {
	threaded_mixer* pMixer = (threaded_mixer*)arg;

	// Set real-time priority for audio thread
	struct sched_param param = {.sched_priority = 90};
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
	
	// Lock memory to prevent swapping
	//mlockall(MCL_CURRENT | MCL_FUTURE);

	struct timespec next_frame;
	clock_gettime(CLOCK_MONOTONIC, &next_frame);
	
	const long frame_ns = (BATCH_SIZE * 1e9) / SAMPLE_RATE;
	
	while (atomic_load(&pMixer->running)) {
		precise_sleep((double)BATCH_SIZE / (double)SAMPLE_RATE);

		/*struct timespec now;*/
		/*clock_gettime(CLOCK_MONOTONIC, &now);*/
		/*long timeout_ns = (next_frame.tv_sec - now.tv_sec) * 1e9 +*/
		/*				 (next_frame.tv_nsec - now.tv_nsec);*/
		/*int timeout_ms = (timeout_ns + 999999) / 1000000;*/
		/*if (timeout_ms < 0) timeout_ms = 0;*/

		pthread_mutex_lock(&pMixer->mutex);
		int current_active = pMixer->active_count;
		pthread_mutex_unlock(&pMixer->mutex);
		
		// Poll all active pipe ends
		/*int ready = poll(pMixer->pfds, pMixer->active_count, timeout_ms);*/
		int ready = poll(pMixer->pfds, current_active, ((double)BATCH_SIZE / (double)SAMPLE_RATE) * 1000 * 2);
		
		// Check poll error
		if (ready == -1) {
			perror("poll failed");
			break;
		}

		if (verbose) {
			for (int i = 0; i < current_active; i++) {
				if (pMixer->pfds[i].revents) {
					   fprintf(stderr, "FD %d: revents = ", pMixer->pfds[i].fd);
					   if (pMixer->pfds[i].revents & POLLIN) fprintf(stderr, "POLLIN ");
					   if (pMixer->pfds[i].revents & POLLHUP) fprintf(stderr, "POLLHUP ");
					   if (pMixer->pfds[i].revents & POLLERR) fprintf(stderr, "POLLERR ");
					   if (pMixer->pfds[i].revents & POLLNVAL) fprintf(stderr, "POLLNVAL ");
					   fprintf(stderr, "\n");
				}
			}
		}

		// Process ready pipes
		if (ready > 0) {
			for (int i = 0; i < current_active && ready > 0; i++) {
				if (pMixer->pfds[i].revents & POLLIN ) {
					int envend = 0;
					
					// Read audio data
					memset(pMixer->temp_buffer, 0, BATCH_SIZE*CHANNELS*sizeof(float));
					ssize_t frames = read_pipe_zero_copy(pMixer->pfds[i].fd, pMixer->temp_buffer, BATCH_SIZE);

					if (verbose) {
						fprintf(stderr,"strumenti:%d\n", frames);
						if (frames != BATCH_SIZE)
							printf("DROP\n");
					}

					if (frames > 0) {
						for (ssize_t iFrame = 0; iFrame < frames; iFrame++) {
							for (ssize_t ci = 0; ci < CHANNELS; ci++) {
								if (isnan(pMixer->temp_buffer[iFrame*CHANNELS + ci])) {
									envend = 1;
									break;
								}
								pMixer->mix_buffer[iFrame*CHANNELS + ci] += pMixer->temp_buffer[iFrame*CHANNELS + ci];
							}
							if (envend) {
								stop_sound_pipe(pMixer, i);//
								i--;  // Adjust index after removal
								break;
							}
						}
					} else if (frames == 0 || (frames == -1 && errno != EAGAIN)) {
						fprintf(stderr, "--pipe err--\n");
						// Pipe closed or error
						stop_sound_pipe(pMixer, i);//
						i--;  // Adjust index after removal
					}
				} else if (pMixer->pfds[i].revents & POLLHUP) {
					fprintf(stderr, "--pollhup--\n");
					stop_sound_pipe(pMixer, i);//
					i--;  // Adjust index after removal
				}
			}

			ready--;
		}

		
		// Time to output mixed buffer
		// if (timeout_ns <= 0) {

		void* pBuffer;
		size_t sizeInBytes = BATCH_SIZE*CHANNELS*sizeof(float);
		ma_rb_acquire_write(pMixer->output_rb, &sizeInBytes, &pBuffer);
		memcpy(pBuffer, pMixer->mix_buffer, sizeInBytes);
		//fprintf(stderr, "%d\n", sizeInBytes);

		ma_rb_commit_write(pMixer->output_rb, sizeInBytes);
			
		// Clear mix buffer for next frame
		memset(pMixer->mix_buffer, 0, BATCH_SIZE*CHANNELS*sizeof(float));
			
			/*// Schedule next frame*/
			/*next_frame.tv_nsec += frame_ns;*/
			/*if (next_frame.tv_nsec >= 1e9) {*/
			/*	next_frame.tv_nsec -= 1e9;*/
			/*	next_frame.tv_sec += 1;*/
			/*}*/
		//}
	}

	return NULL;
}

void parse_skr(track* t) {

	FILE* rfp = fopen(t->file_path, "r");
	if (!rfp) {
		perror("Error opening SKR track file");
		exit(1);
	}

	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	int gathering = 1;
	t->cpm = -1;
	t->rlen = -1;

	while ((read = getline(&line, &len, rfp)) != -1) {
		if (line[0] == '#' || line[0] == ' ' || line[0] == '\n' || len == 1)
			continue;
		if (line[0] == '|')
			gathering = 0;
		if (t->cpm == -1) {
			unsigned int bpm;
			char chars[100];
			sscanf(line, "%u%s", &bpm, chars);
			int nc = 0;
			for (int i = 0; chars[i] != '\0'; i++)
				if (chars[i] == '-')
					nc++;
			if (nc == 0)
				nc = 1;
			t->cpm = (float)bpm * (float)nc;
		} else if (gathering) {
			t->signals[t->si] = (char*)malloc(sizeof(char) * (len-1));
			t->symbols[t->si] = line[0];
			strcat(t->signals[t->si], &line[2]);
			t->signals[t->si][strlen(t->signals[t->si])-1] = '\0';
			fprintf(stderr, "%c %s\n", t->symbols[t->si], t->signals[t->si]);
			t->si++;
		} else {
			if (t->rlen == -1) {
				t->rlen = strchr(&line[1], '|') - line - 1;
				t->rhythm = (char**)malloc(sizeof(char*) * t->rlen);
				for (int i = 0; i < t->rlen; i++) {
					t->rhythm[i] = (char*)malloc(sizeof(char) * 1000);
				}
			} else {
				if (strchr(&line[1], '|') - line - 1 != t->rlen) {
					fprintf(stderr, "Rhythm length mismatch\n");
					exit(1);
				}
			}
			for (size_t i = 1; i < len; i++) {
				if (line[i] == '|')
					break;
				char temp[2] = {line[i], '\0'};
				strcat(t->rhythm[i-1], temp);
			}
		}
	}
	fclose(rfp);
	fprintf(stderr, "timing:%f instruments:%d\n", 60 / t->cpm, t->si);
}

void* sequencer_thread(void* arg) {
	rcomposer* r = (rcomposer*)arg;

	struct stat s;
	stat(r->track_info.file_path, &s);
	time_t mtime = s.st_mtime;

	while (atomic_load(&r->sequencer_running)) {

		for (int i = 0; i < r->track_info.rlen; i++) {
			for (int sj = 0; sj < r->track_info.si; sj++) {

				if (strchr(r->track_info.rhythm[i], r->track_info.symbols[sj])) {


					if (r->pMixer->active_count >= MAX_SOUNDS) {
						continue;
					}
					
					pthread_mutex_lock(&r->pMixer->mutex);

					int idx = r->pMixer->active_count;
					signal_chain* pSound = &r->pMixer->sounds[idx];
					
					// Store chain definition
					pSound->string = strdup(r->track_info.signals[sj]);
					pSound->active = 1;
					
					// Create the final pipe (last process -> mixer)
					if (pipe(pSound->fds) == -1) {
						free(pSound->string);
						pthread_mutex_unlock(&r->pMixer->mutex);
						continue;
					}
					
					// Parse and execute the chain
					if (!execute_signal_chain(pSound)) {
						close(pSound->fds[0]);
						close(pSound->fds[1]);
						free(pSound->string);
						r->pMixer->active_count--;

						pthread_mutex_unlock(&r->pMixer->mutex);
						continue;
					}
					
					// Close write end (used by child processes)
					close(pSound->fds[1]);
					
					// Set up for polling (read end only)
					r->pMixer->pfds[idx].fd = pSound->fds[0];
					r->pMixer->pfds[idx].events = POLLIN;
					fcntl(pSound->fds[0], F_SETFL, O_NONBLOCK);

					r->pMixer->active_count++;
					pthread_mutex_unlock(&r->pMixer->mutex);

					pthread_cond_signal(&r->pMixer->new_sound_cond);
				}
			}

			precise_sleep(60 / r->track_info.cpm);

			stat(r->track_info.file_path, &s);
			if (s.st_mtime > mtime) {
				mtime = s.st_mtime;
				parse_skr(&r->track_info);
			}
		}
	}

	return NULL;
}

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

	ma_rb* pRB = pDevice->pUserData;
	void* pBuffer;
	size_t sizeInBytes = frameCount*CHANNELS*sizeof(float);
	ma_rb_acquire_read(pRB, &sizeInBytes, &pBuffer);
	memcpy(pOutput, pBuffer, sizeInBytes);
	ma_rb_commit_read(pRB, sizeInBytes);
	if (!isatty(1))
		write(1, pOutput, sizeInBytes);
	if (verbose)
		fprintf(stderr, "-----------dc:%d\n", frameCount);

	(void)pInput;
}

int main(int argc, char** argv) {

	// Get args
	struct gengetopt_args_info ai;
	if (cmdline_parser(argc, argv, &ai) != 0) {
		exit(1);
	}
	if (ai.verbose_flag)
		verbose = 1;

	// Create track, parsing file
	track t = { 0 };
	t.file_path = ai.path_arg;
	parse_skr(&t);

	// Create mixer and ring buffer
	threaded_mixer mixer = { 0 };
	ma_rb rb;
	ma_rb_init(2 * SAMPLE_RATE * CHANNELS * sizeof(float), NULL, NULL, &rb);
	mixer.output_rb = &rb;

	// Create rcomposer
	rcomposer r;
	r.pMixer = &mixer;
	r.track_info = t;

	// Init mutexes
	pthread_mutex_init(&mixer.mutex, NULL);
	pthread_cond_init(&mixer.new_sound_cond, NULL);
	atomic_store(&r.sequencer_running, 1);
	atomic_store(&mixer.running, 1);
	
	// Create threads
	pthread_t mixer_tid, sequencer_tid;
	pthread_create(&sequencer_tid, NULL, sequencer_thread, &r);
	pthread_create(&mixer_tid, NULL, mixer_thread, &mixer);

	ma_device device;
	ma_device_config deviceConfig;

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format   = ma_format_f32;
	deviceConfig.playback.channels = CHANNELS;
	deviceConfig.sampleRate = SAMPLE_RATE;
	deviceConfig.dataCallback = data_callback;
	deviceConfig.pUserData = &rb;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		fprintf(stderr, "Failed to open playback device.\n");
		return -4;
	}

	if (isatty(1))
		printf("Device Name: %s\n", device.playback.name);

	if (ma_device_start(&device) != MA_SUCCESS) {
		if (isatty(1)) 
			fprintf(stderr, "Failed to start playback device.\n");
		ma_device_uninit(&device);
		return -5;
	}

	if (isatty(1))
		printf("Press enter to quit...\n");
	getchar();

	atomic_store(&r.sequencer_running, 0);
	atomic_store(&mixer.running, 0);
	
	// Wait for threads
	pthread_join(mixer_tid, NULL);
	pthread_join(sequencer_tid, NULL);
	
	// Cleanup
	pthread_mutex_destroy(&mixer.mutex);
	pthread_cond_destroy(&mixer.new_sound_cond);
	ma_rb_uninit(&rb);
	free(t.rhythm);

	ma_device_uninit(&device);

	return MA_SUCCESS;
}
