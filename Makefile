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

# Tools that depend on command-line parsers
CMD_TOOLS := $(patsubst src/%.ggo,%,$(wildcard src/*.ggo))

# Tool binaries (that don't depend on SDL)
TOOL_EXECS := $(patsubst src/%.tool.c,$(BIN_DIR)/sk.%,$(wildcard src/*.tool.c))

# Specify non-tool, non-command parser objects (with self-named headers)
EXTRA_OBJS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(wildcard src/sk_*.c)) $(OBJ_DIR)/generic_source.o $(OBJ_DIR)/generic_process.o

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
$(foreach tool,$(CMD_TOOLS),$(eval $(OBJ_DIR)/$(tool).tool.o: | $(GEN_DIR)/$(tool).cmdl.h))
$(foreach tool,$(CMD_TOOLS),$(eval $(BIN_DIR)/sk.$(tool): $(OBJ_DIR)/$(tool).cmdl.o))

# Tool build dependencies
$(BIN_DIR)/sk.envelope: $(OBJ_DIR)/sk_adsr.o
$(BIN_DIR)/sk.fmsynth: $(OBJ_DIR)/sk_modwave.o
$(BIN_DIR)/sk.playback: $(OBJ_DIR)/sk_stdins.o
$(BIN_DIR)/sk.decode $(BIN_DIR)/sk.noise $(BIN_DIR)/sk.wave $(BIN_DIR)/sk.fmsynth: $(OBJ_DIR)/generic_source.o 
$(BIN_DIR)/sk.encode $(BIN_DIR)/sk.envelope $(BIN_DIR)/sk.view $(BIN_DIR)/sk.delay $(BIN_DIR)/sk.lpf $(BIN_DIR)/sk.hpf: $(OBJ_DIR)/generic_process.o $(OBJ_DIR)/sk_stdins.o

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
