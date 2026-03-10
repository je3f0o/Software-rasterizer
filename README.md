# Jeefo Canvas Library

A highly optimized, purely software-based rasterizer and graphics library written entirely from scratch in C, developed live on my YouTube stream.

**No OpenGL. No Vulkan. No DirectX. No WebGL.**
Just raw CPU math, memory buffer manipulation, and pure graphics fundamentals.

The entire core of the library sits gracefully at around **700 lines of C code**, making it incredibly lightweight, highly educational, and effortlessly portable.

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
* **Marching Squares**:
  Procedural generation algorithms visualizing complex dynamic fields for showcasing line api.

## 🚀 Portability (Native & Web)

Because this library bypasses operating-system-specific graphics APIs, it can be attached to practically any display layer.
* **Native**: Uses standard `SDL2` merely to present the completed memory buffer to the window context.
* **Web**: Compiles to raw WebAssembly (`wasm32-unknown-unknown`) utilizing a JavaScript wrapper to bridge standard C library functions (`malloc`, `sin`, `cos`) and present the array buffer to an HTML5 Canvas using `putImageData`.

## 🎮 Interactive Web Demos

The `web/` directory contains an elegant, Single-Page Application (SPA) designed to showcase the library natively in your browser using WASM.

**Highlights to check out:**
1. Lines Rasterizer (AA Demo): Demonstrates real-time comparisons between aliased and mathematically smoothed lines using a dual-spinning starburst pattern. Due to browser limitations, the Web Canvas version cannot disable built-in antialiasing.
2. **Cubic & Quadratic Curves:** Interactive demos allowing you to drag control points with your mouse and use the arrow keys to visualize the `T` interpolation variable along the Bezier curve.
3. **Marching Squares:** Generating procedural 2D terrain borders on the fly using line, circle api.

## 🛠️ Building the Project

### Prerequisites
* `clang` (LLVM compiler)
* `wasm-ld` (for WebAssembly linking)
* `pkg-config` and `SDL2` (for Native desktop build examples)

### Compile

To build both the Native binaries and the WebAssembly files, simply run:
```bash
make all