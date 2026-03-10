# Jeefo Canvas Library

A highly optimized, purely software-based rasterizer and graphics library written entirely from scratch in C, developed live on my YouTube stream.

**No OpenGL. No Vulkan. No DirectX. No WebGL.**
Just raw CPU math, memory buffer manipulation, and pure graphics fundamentals pixel by pixel.

The entire core of the library sits gracefully at around **700 lines of C code**, making it incredibly lightweight, very powerful, highly educational, and effortlessly portable.

Check out:

<a href="https://je3f0o.github.io/Software-rasterizer/" target="_blank" rel="noopener">🌐 Try the Interactive Web WASM Demo</a>

---

## ✨ Core Features

* **Software Rasterization Engine**:
  Calculates and paints pixel-perfect primitives directly to a custom byte buffer.
* **Primitive Rendering**:
  Fast 2D and 3D Triangle filling, lines, rectangles, and circles.
* **Advanced Line Algorithms**:
  Bresenham-based native lines and **Xiaolin Wu's Anti-Aliasing** algorithm for beautifully smooth sub-pixel rendering.
* **Math & Curves**:
  Interactive Bezier curve rendering (Quadratic and Cubic curves) utilizing recursive dynamic subdivision.
* **Z-Buffering (Depth Buffer)**:
  Correct 3D depth calculations ensuring objects render perfectly in simulated 3D space.
* **Alpha Blending & Transparency**:
  Built-in color mixing capable of complex semi-transparent overlaps.

---

## 🚀 Portability (Native & Web)

Since Jeefo bypasses OS-specific graphics APIs, it can be attached to almost any display layer.

* **Native** – Uses `SDL2` solely for presenting the completed memory buffer in a window.
* **Web**: Compiles to raw WebAssembly (`wasm32-unknown-unknown`) utilizing a JavaScript wrapper to bridge standard C library functions (`malloc`, `sin`, `cos`) and present the array buffer to an HTML5 Canvas using `putImageData`.

---

## 🛠️ Building the Project

### Prerequisites

* `clang` (LLVM compiler)
* `wasm-ld` (for WebAssembly linking)
* `pkg-config` and `SDL2` (for native desktop build examples)

### Compile

To build **both Native binaries and WebAssembly files**, run:

```bash
make all
```