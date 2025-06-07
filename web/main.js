/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.js
 * Created at  : 2025-06-05
 * Updated at  : 2025-06-08
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
const canvas = document.querySelector("canvas");
const ctx    = canvas.getContext("2d");
canvas.width  = innerWidth;
canvas.height = innerHeight;

(async () => {
  let allocated_memory_size = 0;
  const wasm = await WebAssembly.instantiateStreaming(fetch("lib.wasm"), {
    env: {
      malloc(bytes_size) {
        const {memory, __heap_base} = w.instance.exports;
        const free_memory_size = memory.buffer.byteLength - __heap_base;
        if (bytes_size > free_memory_size) {
          throw new Error("Buy a ram!");
        }

        allocated_memory_size = bytes_size;

        return __heap_base;
      },
      free() {},
      assert(condition) {
        if (!condition) {
          throw new Error("Assert failed");
        }
      },
      stbi_write_png(filename, w, h, comp, data, stride_in_bytes) {
        const {buffer} = wasm.instance.exports.memory;
        const buffer_view = new Uint8ClampedArray(buffer, data, w * h * comp);

        const image_data = new ImageData(buffer_view, w);
        console.log(image_data);
        ctx.putImageData(image_data, 0, 0);
      }
    },
  });
  window.w = wasm;
  wasm.instance.exports.memory.grow(30);

  const {
    create_canvas,
    canvas_draw_scene,
  } = wasm.instance.exports;

  const nc = create_canvas(canvas.width, canvas.height);
  canvas_draw_scene(nc);
  //console.log(nc);

})();