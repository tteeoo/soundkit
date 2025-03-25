CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl -lpthread -lm
TARGET = soundkit
PREFIX = /usr/local

.PHONY: toolchain
toolchain: sk.playback sk.fmsynth

sk.fmsynth: miniaudio/miniaudio.h src/fmsynth.c src/cmads_modwave.c
	${CC} ${CFLAGS} -o sk.fmsynth src/fmsynth.c
sk.playback: miniaudio/miniaudio.h src/playback.c src/cmads_stdins.c
	${CC} ${CFLAGS} -o sk.playback src/playback.c

.PHONY: clean
clean:
	rm -f sk.* src/*.o
