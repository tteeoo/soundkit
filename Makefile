CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
LDFLAGS := -ldl -lpthread -lm

SDL_CFLAGS  := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)


CMDLS_GGOS := $(wildcard src/*.ggo) 
CMDLS_SRCS := $(CMDLS_GGOS:src/%.ggo=src/%.cmdl.h) $(CMDLS_GGOS:src/%.ggo=src/%.cmdl.c)

TOOL_SRCS  := $(wildcard src/*.tool.c)
TOOL_OBJS  := $(TOOL_SRCS:src/%.tool.c=src/%.tool.o)
TOOL_EXECS := $(TOOL_SRCS:src/%.tool.c=build/sk.%)

EXTRA_SRCS := $(wildcard src/cmads_*.c) src/generic_module.c
EXTRA_OBJS := $(EXTRA_SRCS:src/%.c=src/%.o) $(CMDLS_GGOS:src/%.ggo=src/%.cmdl.o)


.PHONY: all toolchain debug clean

all: toolchain

debug: CFLAGS += -DDEBUG -g
debug: all

# Non-tool objects (with headers)
$(EXTRA_OBJS): src/%.o : src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# Generate command line parsers with gengetopt
src/%.cmdl.c src/%.cmdl.h: src/%.ggo
	gengetopt <$^ --file-name=$*.cmdl --output-dir=src/

#
# Toolchain
#
toolchain: $(TOOL_EXECS) build/sk.view

# Specific tool dependencies
src/wave.tool.o: | src/wave.cmdl.h
build/sk.wave: src/generic_module.o src/wave.cmdl.o
build/sk.fmsynth: src/cmads_modwave.o src/generic_module.o
build/sk.playback build/sk.delay build/sk.lpf build/sk.hpf build/sk.encode: src/cmads_stdins.o

# Generic compilation
$(TOOL_OBJS): src/%.tool.o : src/%.tool.c | build
	$(CC) $(CFLAGS) -c $< -o $@
$(TOOL_EXECS): build/sk.% : src/%.tool.o | build
	$(CC) $(LDFLAGS) $^ -o $@ 

# Special handling for SDL requirement
src/sk.view.tool.sdl.o: src/view.tool.sdl.c | build
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $^ -o $@
build/sk.view: src/sk.view.tool.sdl.o src/cmads_stdins.o | build
	$(CC) $(LDFLAGS) $(SDL_LDFLAGS) $^ -o $@


#
# Misc.
#
build:
	mkdir -p build

clean:
	rm -rf build src/*.o src/*.cmdl.*
