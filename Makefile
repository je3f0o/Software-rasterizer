CC        = clang
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

EXAMPLES  =
EXAMPLES += $(BIN_DIR)/line
EXAMPLES += $(BIN_DIR)/transparent
EXAMPLES += $(BIN_DIR)/aa_circle
EXAMPLES += $(BIN_DIR)/2d_triangle
EXAMPLES += $(BIN_DIR)/3d_triangle
EXAMPLES += $(BIN_DIR)/circle

#wasm: $(WEB_DIR)/lib.wasm

.PHONY: examples clean

examples: $(EXAMPLES)

$(BIN_DIR)/%: $(EXAMPLES_DIR)/%.c $(STATIC_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) $(LD_LIBS) $< $(STATIC_LIB) -o $@

#$(WEB_DIR)/lib.wasm: $(WASM_BUILD_DIR)/main.o $(WASM_BUILD_DIR)/lib.o
#wasm-ld -m wasm32 --allow-undefined --export-all --no-entry $^ -o $@

$(NATIVE_BUILD_DIR)/stb_image_write.o: include/stb_image_write.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DSTB_IMAGE_WRITE_IMPLEMENTATION -x c -c $^ -o $@

$(STATIC_LIB): $(NATIVE_BUILD_DIR)/lib.o
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $<

$(NATIVE_BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) $(SDL2_FLAGS) -O2 -c $^ -o $@

$(WASM_BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -O2 -Os --target=wasm32 -nostdlib -c $^ -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)