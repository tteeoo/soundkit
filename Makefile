CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl -lpthread -lm
TARGET = soundkit
PREFIX = /usr/local

.PHONY: toolchain
toolchain: bin/2files

bin/2files: miniaudio/miniaudio.h bin
	${CC} ${CFLAGS} -o bin/2files src/2files.c

bin:
	mkdir bin

.PHONY: clean
clean:
	rm -f bin/* src/*.o
