/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.js
 * Created at  : 2025-06-05
 * Updated at  : 2026-03-11
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/* globals ImageData, WebAssembly */

const demoDefinitions = [
  { id: '2d_triangle', name: '2D Triangle' },
  { id: '3d_triangle', name: '3D Triangle', help: 'Press SPACE to toggle Depth Buffer (Z-Buffer).' },
  { id: 'line', name: 'Lines Rasterizer (AA Demo)', help: 'Left Shape: Aliased | Right Shape: Anti-Aliased' },
  { id: 'cubic_curved_line', name: 'Cubic Curve', help: 'Interactive: Drag nodes. Use Left/Right arrows to move T.' },
  { id: 'quadratic_curved_line', name: 'Quadratic Curve', help: 'Interactive: Drag nodes. Use Left/Right arrows to move T.' },
  { id: 'path', name: 'Cubic & Quad Paths', help: 'Press SPACE to toggle debug bounding lines.' },
  { id: 'circle', name: 'Midpoint Circle' },
  { id: 'aa_circle', name: 'Anti-Aliased Filled Circle' },
  { id: 'transparent', name: 'Transparency 3D Blend' },
  { id: 'marching_squares', name: 'Marching Squares' },
];

const INTERNAL_WIDTH = 800;
const INTERNAL_HEIGHT = 800;

const demosContainer = document.getElementById("demos-container");
const demoStates = {};

// ==========================================
// Function Definitions
// ==========================================
const createEnv = (state) => {
  return {
    cosf: Math.cos,
    sinf: Math.sin,
    floor: Math.floor,
    abs: Math.abs,
    srand: () => {},
    rand01: () => Math.random(),
    malloc: (size) => {
      if (!state.exports) return 0;
      if (state.heapPtr === 0) state.heapPtr = state.exports.__heap_base.value || state.exports.__heap_base;
      const ptr = state.heapPtr;
      state.heapPtr += size;
      state.heapPtr = (state.heapPtr + 7) & ~7; // align to 8 bytes

      const memory = state.exports.memory;
      if (state.heapPtr > memory.buffer.byteLength) {
        memory.grow(Math.ceil((state.heapPtr - memory.buffer.byteLength) / 65536));
      }
      return ptr;
    },
    free: () => {},
    memcpy: (dest, src, size) => {
      if (!state.exports) return;
      const buffer = new Uint8Array(state.exports.memory.buffer);
      buffer.copyWithin(dest, src, src + size);
    },
    memset: (dest, value, size) => {
      if (!state.exports) return;
      const buffer = new Uint8Array(state.exports.memory.buffer);
      buffer.fill(value, dest, dest + size);
    },
    assert: (condition) => {
      if (!condition) throw new Error(`Assert failed in ${state.def.id}`);
    },
    stbi_write_png: () => {}
  };
};

const loadWasm = async (state) => {
  state.overlay.style.display = 'block';

  try {
    const response = await fetch(`${state.def.id}.wasm`);
    if (!response.ok) throw new Error(`Not found`);

    const wasm = await WebAssembly.instantiateStreaming(response, { env: createEnv(state) });
    state.exports = wasm.instance.exports;

    state.canvasStructPtr = state.exports.create_canvas(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    if (state.exports.init_scene) {
      state.exports.init_scene(state.canvasStructPtr);
    }

    state.isLoaded = true;
    state.overlay.style.display = 'none';
    state.lastTimestamp = performance.now();
  } catch (err) {
    console.error(`Failed to load ${state.def.id}:`, err);
    state.overlay.textContent = "Failed to compile/load .wasm";
    state.overlay.style.borderColor = "rgba(255, 100, 100, 0.5)";
    state.overlay.style.color = "#ff8888";
  }
};

const bindInputEvents = (id) => {
  const state = demoStates[id];
  const cvs = state.canvas;

  const getCoords = (e) => {
    const rect = cvs.getBoundingClientRect();
    const touch = e.touches && e.touches.length > 0 ? e.touches[0] : null;
    const clientX = touch ? touch.clientX : e.clientX;
    const clientY = touch ? touch.clientY : e.clientY;
    const x = Math.round(((clientX - rect.left) / rect.width) * INTERNAL_WIDTH);
    const y = Math.round(((clientY - rect.top) / rect.height) * INTERNAL_HEIGHT);
    return { x, y };
  };

  const onDown = (e) => {
    if (state.exports && state.exports.on_mouse_down) {
      const { x, y } = getCoords(e);
      const handled = state.exports.on_mouse_down(x, y);

      if (handled) {
        state.isDragging = true;
        if (e.cancelable) e.preventDefault();
      }
    }
  };

  const onMove = (e) => {
    if (state.isDragging && e.cancelable) {
      e.preventDefault();
    }

    if (state.exports && state.exports.on_mouse_move) {
      const { x, y } = getCoords(e);
      state.exports.on_mouse_move(x, y);
    }
  };

  const onUp = () => {
    state.isDragging = false;
    if (state.exports && state.exports.on_mouse_up) {
      state.exports.on_mouse_up();
    }
  };

  cvs.addEventListener('mousedown', onDown);
  cvs.addEventListener('mousemove', onMove);
  cvs.addEventListener('mouseup', onUp);
  cvs.addEventListener('mouseleave', onUp);

  cvs.addEventListener('touchstart', onDown, { passive: false });
  cvs.addEventListener('touchmove', onMove, { passive: false });
  cvs.addEventListener('touchend', onUp);
  cvs.addEventListener('touchcancel', onUp);
};

const globalRenderLoop = (timestamp) => {
  for (const id in demoStates) {
    const state = demoStates[id];

    // Skip canvases that are off-screen or haven't loaded yet
    if (!state.isVisible || !state.isLoaded) continue;

    // Handle large dt jumps when unpausing
    if (state.lastTimestamp === 0 || timestamp - state.lastTimestamp > 100) {
      state.lastTimestamp = timestamp;
    }
    const dt = (timestamp - state.lastTimestamp) * 0.001;
    state.lastTimestamp = timestamp;

    // FPS Counter
    state.frameCount++;
    if (timestamp - state.lastFpsTime >= 1000) {
      state.fpsEl.textContent = `${state.frameCount} FPS`;
      state.frameCount = 0;
      state.lastFpsTime = timestamp;
    }

    const exp = state.exports;
    if (exp.canvas_update) exp.canvas_update(state.canvasStructPtr, dt);
    if (exp.canvas_render) exp.canvas_render(state.canvasStructPtr);

    // Blit Memory to Canvas
    if (exp.memory) {
      const canvasStructView = new Uint32Array(exp.memory.buffer, state.canvasStructPtr, 4);
      const colorBufferPtr = canvasStructView[2];

      if (colorBufferPtr > 0) {
        const pixelsView = new Uint8ClampedArray(
          exp.memory.buffer,
          colorBufferPtr,
          INTERNAL_WIDTH * INTERNAL_HEIGHT * 4
        );
        state.ctx.putImageData(new ImageData(pixelsView, INTERNAL_WIDTH, INTERNAL_HEIGHT), 0, 0);
      }
    }
  }

  requestAnimationFrame(globalRenderLoop);
};

// ==========================================
// Setup & Initialization
// ==========================================

// Global Keyboard Handler (Window level)
window.addEventListener('keydown', (e) => {
  const usedKeys = [32, 37, 39]; // Space, Left Arrow, Right Arrow
  let keyHandled = false;

  // Dispatch key event to ALL visible demos
  for (const id in demoStates) {
    const state = demoStates[id];
    if (state.isVisible && state.exports && state.exports.on_key_down) {
      state.exports.on_key_down(e.keyCode);

      // If a visible demo uses this key, mark it to prevent default scrolling
      if (usedKeys.includes(e.keyCode)) {
        keyHandled = true;
      }
    }
  }

  // Prevent default page scroll if interacting with a visible demo
  if (keyHandled) {
    e.preventDefault();
  }
});

// Intersection Observer for Lazy Loading and Pausing
const observer = new IntersectionObserver((entries) => {
  entries.forEach(entry => {
    const id = entry.target.dataset.id;
    const state = demoStates[id];

    if (entry.isIntersecting) {
      state.isVisible = true;
      if (!state.isLoaded && !state.isLoading) {
        state.isLoading = true;
        loadWasm(state);
      }
    } else {
      state.isVisible = false;
    }
  });
}, { rootMargin: "100px 0px" });

// Build the UI for each demo
demoDefinitions.forEach(def => {
  const card = document.createElement("div");
  card.className = "demo-card";
  card.dataset.id = def.id;

  const helpHTML = def.help ? `<div class="instruction-bar"><span class="icon">💡</span> <span>${def.help}</span></div>` : '';

  card.innerHTML = `
    <div class="demo-header">
      <div class="demo-title">${def.name}</div>
      <div class="demo-fps" id="fps-${def.id}">0 FPS</div>
    </div>
    ${helpHTML}
    <div class="canvas-wrapper">
      <canvas id="canvas-${def.id}" width="${INTERNAL_WIDTH}" height="${INTERNAL_HEIGHT}"></canvas>
      <div class="canvas-overlay" id="overlay-${def.id}">Loading...</div>
    </div>
  `;
  demosContainer.appendChild(card);

  const canvas = document.getElementById(`canvas-${def.id}`);
  demoStates[def.id] = {
    def,
    canvas,
    ctx: canvas.getContext("2d", { alpha: false }),
    overlay: document.getElementById(`overlay-${def.id}`),
    fpsEl: document.getElementById(`fps-${def.id}`),
    exports: null,
    canvasStructPtr: 0,
    isLoaded: false,
    isLoading: false,
    isVisible: false,
    isDragging: false,
    heapPtr: 0,
    lastTimestamp: 0,
    frameCount: 0,
    lastFpsTime: 0
  };

  bindInputEvents(def.id);
  observer.observe(card);
});

// Start the global loop once
requestAnimationFrame(globalRenderLoop);