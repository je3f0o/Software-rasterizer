/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.js
 * Created at  : 2025-06-05
 * Updated at  : 2025-06-11
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
const canvas = document.querySelector("canvas");
const ctx    = canvas.getContext("2d");
canvas.width  = 50;
canvas.height = 50;

canvas.style.width  = "100%";
canvas.style.height = "100%";

(async () => {
  let allocated_memory_size = 0;
  const wasm = await WebAssembly.instantiateStreaming(fetch("lib.wasm"), {
    env: {
      cosf: (angle) => {
        //console.log(angle);
        return Math.cos(angle);
      },
      sinf(angle) {
        console.log("sin", angle);
        return Math.sin(angle);
      },
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
        ctx.putImageData(new ImageData(buffer_view, w), 0, 0);
      }
    },
  });
  window.w = wasm;
  wasm.instance.exports.memory.grow(30);

  const {
    create_canvas,
    init_scene,
    canvas_update,
    canvas_render,
  } = wasm.instance.exports;

  const nc = create_canvas(50, 50);
  init_scene(nc);
  canvas_render(nc);

  const dt = 1000/60;

  canvas.addEventListener("click", () => {
    canvas_update(nc, dt)
    canvas_render(nc);
  });

  //let last_timestamp = 0;
  //requestAnimationFrame(function loop(timestamp) {
  //  const dt = timestamp - last_timestamp;
  //  last_timestamp = timestamp;

  //  requestAnimationFrame(loop);
  //});

})();