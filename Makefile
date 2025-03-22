CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl -lpthread -lm
TARGET = soundkit
PREFIX = /usr/local

.PHONY: toolchain
toolchain: sk.2files sk.fmsynth

sk.fmsynth: miniaudio/miniaudio.h src/fmsynth.c src/cmads_modwave.c
	${CC} ${CFLAGS} -o sk.fmsynth src/fmsynth.c
sk.2files: miniaudio/miniaudio.h src/2files.c 
	${CC} ${CFLAGS} -o sk.2files src/2files.c

.PHONY: clean
clean:
	rm -f sk.* src/*.o
