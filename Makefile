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

# Tool binaries (that don't depend on SDL)
TOOL_EXECS := $(patsubst src/%.tool.c,$(BIN_DIR)/sk.%,$(wildcard src/*.tool.c))

# Specify non-tool, non-command parser objects (with self-named headers)
EXTRA_OBJS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(wildcard src/cmads_*.c)) $(OBJ_DIR)/generic_source.o $(OBJ_DIR)/generic_process.o

# Default to toolchain
.PHONY:
all: toolchain

# Phony target to build all tools
.PHONY:
toolchain: $(TOOL_EXECS) $(BIN_DIR)/sk.view
.PHONY:
toolchain-nosdl: $(TOOL_EXECS)

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

# Tool command-line dependencies
$(OBJ_DIR)/decode.tool.o: | $(GEN_DIR)/decode.cmdl.h
$(OBJ_DIR)/wave.tool.o: | $(GEN_DIR)/wave.cmdl.h
$(OBJ_DIR)/noise.tool.o: | $(GEN_DIR)/noise.cmdl.h
$(OBJ_DIR)/envelope.tool.o: | $(GEN_DIR)/envelope.cmdl.h
$(BIN_DIR)/sk.decode: $(OBJ_DIR)/decode.cmdl.o
$(BIN_DIR)/sk.noise: $(OBJ_DIR)/noise.cmdl.o
$(BIN_DIR)/sk.wave: $(OBJ_DIR)/wave.cmdl.o
$(BIN_DIR)/sk.envelope: $(OBJ_DIR)/envelope.cmdl.o

# Tool build dependencies
$(BIN_DIR)/sk.fmsynth: $(OBJ_DIR)/cmads_modwave.o
$(BIN_DIR)/sk.playback: $(OBJ_DIR)/cmads_stdins.o
$(BIN_DIR)/sk.decode $(BIN_DIR)/sk.noise $(BIN_DIR)/sk.wave $(BIN_DIR)/sk.fmsynth: $(OBJ_DIR)/generic_source.o 
$(BIN_DIR)/sk.envelope $(BIN_DIR)/sk.view $(BIN_DIR)/sk.delay $(BIN_DIR)/sk.lpf $(BIN_DIR)/sk.hpf: $(OBJ_DIR)/generic_process.o $(OBJ_DIR)/cmads_stdins.o

# Generic tool compilation
$(OBJ_DIR)/%.tool.o: src/%.tool.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(GEN_DIR) -c $< -o $@
$(TOOL_EXECS): $(BIN_DIR)/sk.% : $(OBJ_DIR)/%.tool.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@ 

# Special handling for SDL requirement
$(OBJ_DIR)/view.tool.sdl.o: src/view.tool.sdl.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $^ -o $@
$(BIN_DIR)/sk.view: $(OBJ_DIR)/view.tool.sdl.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $(SDL_LDFLAGS) $^ -o $@
