/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.js
 * Created at  : 2025-06-05
 * Updated at  : 2025-06-13
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
const canvas = document.querySelector("canvas");
const ctx    = canvas.getContext("2d");
canvas.width  = window.innerWidth;
canvas.height = window.innerHeight;

const PAGE_SIZE = 64 * 1024;

(async () => {
  let allocated_memory_size = 0;
  const wasm = await WebAssembly.instantiateStreaming(fetch("lib.wasm"), {
    env: {
      cosf: Math.cos,
      sinf: Math.sin,
      malloc(bytes_size) {
        const {memory, __heap_base} = w.instance.exports;
        const free_memory_size = memory.buffer.byteLength - __heap_base;
        if (bytes_size > free_memory_size) {
          const bytes_needed = bytes_size - free_memory_size;
          const pages = Math.ceil(bytes_needed / PAGE_SIZE);
          memory.grow(pages);
        }

        allocated_memory_size = bytes_size;
        return __heap_base;
      },
      free() {},
      memcpy(dest, src, size) {
        const {buffer} = wasm.instance.exports.memory;
        const dest_buffer = new Uint8ClampedArray(buffer, dest, size);
        const src_buffer  = new Uint8ClampedArray(buffer, src, size);

        for (let i = 0; i < size; ++i) {
          dest_buffer[i] = src_buffer[i];
        }
      },
      memset(dest, value, size) {
        const {buffer} = wasm.instance.exports.memory;
        const dest_buffer = new Uint8ClampedArray(buffer, dest, size);
        for (let i = 0; i < size; ++i) {
          dest_buffer[i] = value;
        }
      },
      assert(condition) {
        if (!condition) {
          throw new Error("Assert failed");
        }
      },
      stbi_write_png(filename, w, h, comp, data, stride_in_bytes) {
        const {buffer} = wasm.instance.exports.memory;
        const buffer_view = new Uint8ClampedArray(buffer, data, w * h * comp);
        ctx.putImageData(new ImageData(buffer_view, w), 0, 0);
      }
    },
  });
  window.w = wasm;

  const {
    create_canvas,
    init_scene_2d,
    canvas_update_2d,
    canvas_render_2d,
    init_scene_3d,
    canvas_update_3d,
    canvas_render_3d,
  } = wasm.instance.exports;

  const nc = create_canvas(canvas.width, canvas.height);
  init_scene_2d(nc);
  init_scene_3d(nc);

  let last_timestamp = 0;
  requestAnimationFrame(function loop(timestamp) {
    const dt = (timestamp - last_timestamp) * 0.001;
    last_timestamp = timestamp;

    canvas_update_3d(nc, dt);
    canvas_render_3d(nc);

    requestAnimationFrame(loop);
  });

})();