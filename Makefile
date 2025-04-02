CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -ldl -lpthread -lm
SDLFLAGS = $(shell sdl2-config --cflags --libs)

TOOL_SOURCES = $(wildcard src/*.tool.c)
TOOL_EXECS = $(TOOL_SOURCES:src/%.tool.c=build/sk.%)

.PHONY: all toolchain clean

all: toolchain

toolchain: $(TOOL_EXECS) build/sk.view

build/sk.fmsynth: src/cmads_modwave.c
build/sk.playback build/sk.delay build/sk.lpf build/sk.hpf: src/cmads_stdins.c

build/sk.view: src/view.tool.sdl.c | build
	$(CC) $(CFLAGS) $(SDLFLAGS) $< -o $@ $(LDFLAGS)

$(TOOL_EXECS): build/sk.% : src/%.tool.c | build
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

build:
	mkdir -p build

clean:
	rm -rf build src/*.o
