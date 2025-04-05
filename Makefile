# Target directory configuration
BUILD_DIR := build
GEN_DIR   := $(BUILD_DIR)/gen
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin

# Compilation configuration
CC          := gcc
CFLAGS      := -Wall -Wextra -std=c11
LDFLAGS     := -ldl -lpthread -lm
SDL_CFLAGS  := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

# Command-line parser source code generated with getgenopt
CMDLS_GGOS := $(wildcard src/*.ggo) 
CMDLS_SRCS := $(CMDLS_GGOS:src/%.ggo=$(GEN_DIR)/%.cmdl.h) $(CMDLS_GGOS:src/%.ggo=$(GEN_DIR)/%.cmdl.c)

# Tool binaries (that don't depend on SDL)
TOOL_SRCS  := $(wildcard src/*.tool.c)
TOOL_OBJS  := $(TOOL_SRCS:src/%.tool.c=$(OBJ_DIR)/%.tool.o)
TOOL_EXECS := $(TOOL_SRCS:src/%.tool.c=$(BIN_DIR)/sk.%)

# Non-tool objects (with self-named headers)
EXTRA_SRCS := $(wildcard src/cmads_*.c) src/generic_module.c
EXTRA_OBJS := $(EXTRA_SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Default to toolchain
.PHONY:
all: toolchain

# Phony target to build all tools
.PHONY:
toolchain: $(TOOL_EXECS) $(BIN_DIR)/sk.view

# Phony target to enable debug compilation
.PHONY:
debug: CFLAGS += -DDEBUG -g
debug: all

# Phony target to clean up files
.PHONY:
clean:
	rm -rf build

# Ensure directories exist
$(GEN_DIR) $(OBJ_DIR) $(BIN_DIR): | $(BUILD_DIR)
$(GEN_DIR) $(OBJ_DIR) $(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

# Non-tool objects with self-named headers
$(EXTRA_OBJS): $(OBJ_DIR)/%.o : src/%.c src/%.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Generate command line parsers with gengetopt
$(GEN_DIR)/%.cmdl.c $(GEN_DIR)/%.cmdl.h: src/%.ggo | $(GEN_DIR)
	gengetopt <$^ --file-name=$*.cmdl --output-dir=$(GEN_DIR)

# Command-line parser objects
$(OBJ_DIR)/%.cmdl.o: $(GEN_DIR)/%.cmdl.c $(GEN_DIR)/%.cmdl.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Specific tool dependencies
$(OBJ_DIR)/wave.tool.o: | $(GEN_DIR)/wave.cmdl.h
$(BIN_DIR)/sk.wave: $(OBJ_DIR)/generic_module.o $(OBJ_DIR)/wave.cmdl.o
$(BIN_DIR)/sk.fmsynth: $(OBJ_DIR)/cmads_modwave.o $(OBJ_DIR)/generic_module.o
$(BIN_DIR)/sk.playback $(BIN_DIR)/sk.delay $(BIN_DIR)/sk.lpf $(BIN_DIR)/sk.hpf $(BIN_DIR)/sk.encode: $(OBJ_DIR)/cmads_stdins.o

# Generic tool compilation
$(TOOL_OBJS): $(OBJ_DIR)/%.tool.o : src/%.tool.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(GEN_DIR) -c $< -o $@
$(TOOL_EXECS): $(BIN_DIR)/sk.% : $(OBJ_DIR)/%.tool.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@ 

# Special handling for SDL requirement
$(OBJ_DIR)/view.tool.sdl.o: src/view.tool.sdl.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $^ -o $@
$(BIN_DIR)/sk.view: $(OBJ_DIR)/view.tool.sdl.o $(OBJ_DIR)/cmads_stdins.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $(SDL_LDFLAGS) $^ -o $@
