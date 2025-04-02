CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -g
LDFLAGS := -ldl -lpthread -lm

SDL_CFLAGS  := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

TOOL_SOURCES := $(wildcard src/*.tool.c)
TOOL_EXECS   := $(TOOL_SOURCES:src/%.tool.c=build/sk.%)

.PHONY: all toolchain clean

all: toolchain

toolchain: $(TOOL_EXECS) build/sk.view

build/sk.fmsynth: src/cmads_modwave.c
build/sk.playback build/sk.delay build/sk.lpf build/sk.hpf: src/cmads_stdins.c

$(TOOL_EXECS): build/sk.% : src/%.tool.c | build
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

build/sk.view: src/view.tool.sdl.c src/cmads_stdins.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $< -o $@ $(LDFLAGS) $(SDL_LDFLAGS)

build:
	mkdir -p build

clean:
	rm -rf build src/*.o
