CC        = clang
CFLAGS    = -Wall -Wextra -g
HEADERS   = -I./include

WEB_DIR          = web
WASM_BUILD_DIR   = build/wasm
NATIVE_BUILD_DIR = build/native

run: rasterizer
	./rasterizer && feh --auto-zoom image.png

rasterizer: $(NATIVE_BUILD_DIR)/main.o $(NATIVE_BUILD_DIR)/lib.o $(NATIVE_BUILD_DIR)/stb_image_write.o
	$(CC) $(CFLAGS) $(HEADERS) $^ -o $@

wasm: $(WEB_DIR)/lib.wasm

$(WEB_DIR)/lib.wasm: $(WASM_BUILD_DIR)/main.o $(WASM_BUILD_DIR)/lib.o
	wasm-ld -m wasm32 --allow-undefined --export-all --no-entry $^ -o $@

$(NATIVE_BUILD_DIR)/stb_image_write.o: include/stb_image_write.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DSTB_IMAGE_WRITE_IMPLEMENTATION -x c -c $^ -o $@

$(NATIVE_BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -c $^ -o $@

$(WASM_BUILD_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(HEADERS) -Os -O3 --target=wasm32 -nostdlib -c $^ -o $@

clear:
	rm -rf build