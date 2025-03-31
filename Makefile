CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -ldl -lpthread -lm

TOOL_SOURCES = $(wildcard src/*.tool.c)
TOOL_EXECS = $(TOOL_SOURCES:src/%.tool.c=build/sk.%)

.PHONY: all toolchain clean

all: toolchain

toolchain: $(TOOL_EXECS)

src/fmsynth.tool.c: src/cmads_modwave.c
src/playback.tool.c src/delay.tool.c: src/cmads_modwave.c

$(TOOL_EXECS): build/sk.% : src/%.tool.c | build
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build:
	mkdir -p build

clean:
	rm -rf build src/*.o
