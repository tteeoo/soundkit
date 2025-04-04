CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
LDFLAGS := -ldl -lpthread -lm

SDL_CFLAGS  := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

CMADS_SRCS := $(wildcard src/cmads_*.c)
CMADS_OBJS := $(CMADS_SRCS:src/%.c=src/%.o)

TOOL_SRCS  := $(wildcard src/*.tool.c)
TOOL_OBJS  := $(TOOL_SRCS:src/%.tool.c=src/%.tool.o)
TOOL_EXECS := $(TOOL_SRCS:src/%.tool.c=build/sk.%)

.PHONY: all toolchain debug clean

all: toolchain

debug: CFLAGS += -DDEBUG -g
debug: all

# Generic module object
src/generic_module.o: src/generic_module.c src/generic_module.h | build
	$(CC) $(CFLAGS) -c $< -o $@

# Custom miniaudio data source generic compilation
$(CMADS_OBJS): src/%.o : src/%.c src/%.h | build
	$(CC) $(CFLAGS) -c $< -o $@

#
# Toolchain
#
toolchain: $(TOOL_EXECS) build/sk.view

# Specific tool dependencies
build/sk.fmsynth: src/cmads_modwave.o src/generic_module.o
build/sk.playback build/sk.delay build/sk.lpf build/sk.hpf build/sk.encode: src/cmads_stdins.o

# Generic compilation
$(TOOL_OBJS): src/%.tool.o : src/%.tool.c | build
	$(CC) $(CFLAGS) -c $^ -o $@
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
	rm -rf build src/*.o
