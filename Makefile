#CC        = clang
CC        = /usr/local/Cellar/llvm/22.1.0/bin/clang
AR        = ar
CFLAGS    = -Wall -Wextra
LD_LIBS   = -lm $(shell pkg-config --libs sdl2)
HEADERS   = -I include

SDL2_FLAGS = $(shell pkg-config --cflags sdl2)

WEB_DIR          = web
BIN_DIR          = bin
BUILD_DIR        = build
EXAMPLES_DIR     = examples
WASM_BUILD_DIR   = $(BUILD_DIR)/wasm
NATIVE_BUILD_DIR = $(BUILD_DIR)/native

STATIC_LIB = $(BUILD_DIR)/canvas.a

# List of all your examples (removed path_close)
EXAMPLE_NAMES =

EXAMPLE_NAMES += marching_squares
EXAMPLE_NAMES += line
EXAMPLE_NAMES += transparent
EXAMPLE_NAMES += aa_circle
EXAMPLE_NAMES += 2d_triangle
EXAMPLE_NAMES += 3d_triangle
EXAMPLE_NAMES += circle
EXAMPLE_NAMES += quadratic_curved_line
EXAMPLE_NAMES += cubic_curved_line
EXAMPLE_NAMES += path

EXAMPLES      = $(addprefix $(BIN_DIR)/, $(EXAMPLE_NAMES))
WASM_EXAMPLES = $(addprefix $(WEB_DIR)/, $(addsuffix .wasm, $(EXAMPLE_NAMES)))

.PHONY: all examples wasm clean

all: examples wasm

examples: $(EXAMPLES)
wasm: $(WASM_EXAMPLES)

# --- Native Build Rules ---

$(BIN_DIR)/%: $(EXAMPLES_DIR)/%.c $(STATIC_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) $(LD_LIBS) $< $(STATIC_LIB) -o $@

$(NATIVE_BUILD_DIR)/stb_image_write.o: include/stb_image_write.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DSTB_IMAGE_WRITE_IMPLEMENTATION -x c -c $^ -o $@

$(STATIC_LIB): $(NATIVE_BUILD_DIR)/lib.o
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $<

$(NATIVE_BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) $(SDL2_FLAGS) -O2 -c $^ -o $@

# --- WebAssembly Build Rules ---

$(WASM_BUILD_DIR)/lib.o: src/lib.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -O2 -Os --target=wasm32 -nostdlib -c $< -o $@

$(WASM_BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -O2 -Os --target=wasm32 -nostdlib -c $< -o $@

$(WEB_DIR)/%.wasm: $(WASM_BUILD_DIR)/%.o $(WASM_BUILD_DIR)/lib.o
	@mkdir -p $(dir $@)
	wasm-ld -m wasm32 --allow-undefined --export-all --no-entry $^ -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(WEB_DIR)/*.wasm