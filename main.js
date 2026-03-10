/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.js
 * Created at  : 2025-06-05
 * Updated at  : 2026-03-10
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/* globals ImageData, WebAssembly */

const demos = [
  { id: '2d_triangle', name: '2D Triangle' },
  { id: '3d_triangle', name: '3D Triangle', help: 'Press SPACE to toggle between Color and Depth Buffer (Z-Buffer).' },
  { id: 'line', name: 'Lines Rasterizer (AA Demo)', help: 'Left Shape: Aliased Rendering | Right Shape: Anti-Aliased Rendering' },
  { id: 'cubic_curved_line', name: 'Cubic Curve', help: 'Interactive: Drag nodes with mouse. Use Left/Right arrows to move T.' },
  { id: 'quadratic_curved_line', name: 'Quadratic Curve', help: 'Interactive: Drag nodes with mouse. Use Left/Right arrows to move T.' },
  { id: 'path', name: 'Cubic & Quad Paths', help: 'Press SPACE to toggle debug bounding lines.' },
  { id: 'circle', name: 'Midpoint Circle' },
  { id: 'aa_circle', name: 'Anti-Aliased Filled Circle' },
  { id: 'transparent', name: 'Transparency 3D Blend' },
  { id: 'marching_squares', name: 'Marching Squares' },
];

const viewCanvas = document.getElementById("view-canvas");
const ctx = viewCanvas.getContext("2d", { alpha: false });
const demoListEl = document.getElementById("demo-list");
const demoTitleEl = document.getElementById("demo-title");
const overlayMsg = document.getElementById("overlay-message");
const fpsCounter = document.getElementById("fps-counter");

const instructionBar = document.getElementById("instruction-bar");
const instructionText = document.getElementById("instruction-text");

const INTERNAL_WIDTH = 800;
const INTERNAL_HEIGHT = 800;
viewCanvas.width = INTERNAL_WIDTH;
viewCanvas.height = INTERNAL_HEIGHT;

let currentRafId = null;
let activeExports = null;

const showMessage = (msg, isError = false) => {
  overlayMsg.textContent = msg;
  overlayMsg.style.display = 'block';
  overlayMsg.className = isError ? '' : 'loading';
  if (isError) {
    ctx.fillStyle = '#181818';
    ctx.fillRect(0, 0, viewCanvas.width, viewCanvas.height);
  }
};

const hideMessage = () => {
  overlayMsg.style.display = 'none';
};

const updateHelpText = (text) => {
  if (text) {
    instructionText.textContent = text;
    instructionBar.style.display = 'flex';
  } else {
    instructionBar.style.display = 'none';
  }
};

const handleMouseMove = (e) => {
  if (!activeExports || !activeExports.on_mouse_move) return;
  const rect = viewCanvas.getBoundingClientRect();
  const x = Math.round(((e.clientX - rect.left) / rect.width) * INTERNAL_WIDTH);
  const y = Math.round(((e.clientY - rect.top) / rect.height) * INTERNAL_HEIGHT);
  activeExports.on_mouse_move(x, y);
};

const handleMouseDown = (e) => {
  if (!activeExports || !activeExports.on_mouse_down) return;
  viewCanvas.focus(); // Ensure canvas captures key events when clicked
  const rect = viewCanvas.getBoundingClientRect();
  const x = Math.round(((e.clientX - rect.left) / rect.width) * INTERNAL_WIDTH);
  const y = Math.round(((e.clientY - rect.top) / rect.height) * INTERNAL_HEIGHT);
  activeExports.on_mouse_down(x, y);
};

const handleMouseUp = () => {
  if (activeExports && activeExports.on_mouse_up) activeExports.on_mouse_up();
};

const handleKeyDown = (e) => {
  // Prevent default scrolling for arrows and space when interacting
  if ([32, 37, 38, 39, 40].includes(e.keyCode)) {
    e.preventDefault();
  }
  activeExports.on_key_down?.(e.keyCode);
};

window.addEventListener('mousemove', handleMouseMove);
window.addEventListener('mousedown', handleMouseDown);
window.addEventListener('mouseup', handleMouseUp);
window.addEventListener('mouseleave', handleMouseUp);
window.addEventListener('keydown', handleKeyDown);

const loadDemo = async (demo) => {
  if (currentRafId) cancelAnimationFrame(currentRafId);

  document.querySelectorAll('.demo-btn').forEach(btn => btn.classList.remove('active'));
  document.getElementById(`btn-${demo.id}`).classList.add('active');
  demoTitleEl.textContent = demo.name;

  updateHelpText(demo.help);
  showMessage(`Loading ${demo.name}...`);

  let heapPtr = 0;
  activeExports = null;

  const env = {
    cosf: Math.cos,
    sinf: Math.sin,
    floor: Math.floor,
    abs: Math.abs,
    srand: () => {},
    rand01: () => Math.random(),
    malloc: (size) => {
      if (!activeExports) return 0;
      if (heapPtr === 0) heapPtr = activeExports.__heap_base.value || activeExports.__heap_base;
      const ptr = heapPtr;
      heapPtr += size;
      heapPtr = (heapPtr + 7) & ~7;

      const memory = activeExports.memory;
      if (heapPtr > memory.buffer.byteLength) {
        memory.grow(Math.ceil((heapPtr - memory.buffer.byteLength) / 65536));
      }
      return ptr;
    },
    free: () => {},
    memcpy: (dest, src, size) => {
      if (!activeExports) return;
      const buffer = new Uint8Array(activeExports.memory.buffer);
      buffer.copyWithin(dest, src, src + size);
    },
    memset: (dest, value, size) => {
      if (!activeExports) return;
      const buffer = new Uint8Array(activeExports.memory.buffer);
      buffer.fill(value, dest, dest + size);
    },
    assert: (condition) => {
      if (!condition) throw new Error("Assert failed");
    },
    stbi_write_png: () => {}
  };

  try {
    const response = await fetch(`${demo.id}.wasm`);
    if (!response.ok) throw new Error(`WASM file not found: ${demo.id}.wasm`);

    const wasm = await WebAssembly.instantiateStreaming(response, { env });
    activeExports = wasm.instance.exports;

    const nc = activeExports.create_canvas(INTERNAL_WIDTH, INTERNAL_HEIGHT);

    if (activeExports.init_scene) {
      activeExports.init_scene(nc);
    }

    let lastTimestamp = performance.now();
    let frameCount = 0;
    let lastFpsTime = lastTimestamp;

    hideMessage();

    const loop = (timestamp) => {
      const dt = (timestamp - lastTimestamp) * 0.001;
      lastTimestamp = timestamp;

      frameCount++;
      if (timestamp - lastFpsTime >= 1000) {
        fpsCounter.textContent = `${frameCount} FPS`;
        frameCount = 0;
        lastFpsTime = timestamp;
      }

      if (activeExports.canvas_update) activeExports.canvas_update(nc, dt);
      if (activeExports.canvas_render) activeExports.canvas_render(nc);

      if (activeExports.memory) {
        // Canvas struct layout: width(0), height(4), color_buffer_ptr(8)
        const canvasStructView = new Uint32Array(activeExports.memory.buffer, nc, 4);
        const colorBufferPtr = canvasStructView[2];

        if (colorBufferPtr > 0) {
          const pixelsView = new Uint8ClampedArray(
            activeExports.memory.buffer,
            colorBufferPtr,
            INTERNAL_WIDTH * INTERNAL_HEIGHT * 4
          );
          ctx.putImageData(new ImageData(pixelsView, INTERNAL_WIDTH, INTERNAL_HEIGHT), 0, 0);
        }
      }

      currentRafId = requestAnimationFrame(loop);
    };

    currentRafId = requestAnimationFrame(loop);

  } catch (err) {
    console.error(err);
    showMessage(`Failed to load ${demo.name}\nMake sure ${demo.id}.wasm is compiled.`, true);
  }
};

demos.forEach(demo => {
  const btn = document.createElement('button');
  btn.className = 'demo-btn';
  btn.id = `btn-${demo.id}`;
  btn.textContent = demo.name;
  btn.onclick = () => loadDemo(demo);
  demoListEl.appendChild(btn);
});

if (demos.length > 0) {
  loadDemo(demos[0]);
}