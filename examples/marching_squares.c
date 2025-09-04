/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : marching_squares.c
 * Created at  : 2025-07-11
 * Updated at  : 2025-09-05
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include "lib.h"

#if defined(__wasm__) || defined(__wasm32__)
int   stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
#else
#include <SDL2/SDL.h>
#include <assert.h>
#include "stb_image_write.h"
#endif

#ifndef M_PIf
  #define M_PIf	3.14159265358979323846f	/* pi */
#endif
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800
#define CANVAS_WIDTH  800
#define CANVAS_HEIGHT 800

#define RES      20
#define HALF_RES (RES / 2)

struct {
  u32    rows;
  u32    cols;
  float* data;
} grid = {0};

void canvas_present(Canvas* canvas) {
  assert(canvas != null);
#if defined(__wasm__) || defined(__wasm32__) || defined(PLATFORM_IMAGE)
  stbi_write_png("image.png", canvas->width, canvas->height, 4, canvas->color_buffer, 0);
#elif defined(PLATFORM_SDL)
#endif
}

INLINE float rand01(void) {
  return rand()/(float)RAND_MAX;
}

INLINE int is_rock(u32 x, u32 y) {
  u32 index = y * grid.cols + x;
  return grid.data[index] > 0.5 ? 1 : 0;
}

INLINE i32 get_state(int a, int b, int c, int d) {
  return a*8 + b*4 + c*2 + d;
}

void init_scene(Canvas* canvas) {
  UNUSED(canvas);

  srand(69);

  u32 cols = canvas->width  / RES + 1;
  u32 rows = canvas->height / RES + 1;

  grid.rows = rows;
  grid.cols = cols;
  grid.data = malloc(rows*cols*sizeof(float));
  for (u32 i = 0; i < rows; ++i) {
    for (u32 j = 0; j < cols; ++j) {
      u32 index = i*cols + j;
      grid.data[index] = rand01();
    }
  }
}

void canvas_update(Canvas* canvas, double dt) {
  UNUSED(dt);
  UNUSED(canvas);
}

void canvas_render(Canvas* canvas) {
  canvas_clear(canvas, GRAY);

  Line   line   = {.color = WHITE, .antialiased = true};
  Circle circle = {.radius = RES/5};
  for (u32 y = 0; y < grid.rows; ++y) {
    for (u32 x = 0; x < grid.cols; ++x) {
      circle.x = x * RES;
      circle.y = y * RES;

      Color color = is_rock(x, y) == 1 ? RED : GREEN;
      canvas_fill_circle(canvas, circle, color);

      i32 tl = is_rock(x   , y);
      i32 tr = is_rock(x+1 , y);
      i32 br = is_rock(x+1 , y+1);
      i32 bl = is_rock(x   , y+1);

      ivec2 top    = {(x+0) * RES + HALF_RES , (y+0) * RES            };
      ivec2 left   = {(x+0) * RES            , (y+0) * RES + HALF_RES };
      ivec2 right  = {(x+1) * RES            , (y+0) * RES + HALF_RES };
      ivec2 bottom = {(x+0) * RES + HALF_RES , (y+1) * RES            };

      i32 state = get_state(tl, tr, br, bl);
      if (state == 0 || state == 15) continue;

      switch (state) {
        case 1:
        case 14:
          line.to   = bottom;
          line.from = left;
          break;
        case 2:
        case 13:
          line.to   = bottom;
          line.from = right;
          break;
        case 3:
        case 12:
          line.to   = left;
          line.from = right;
          break;
        case 4:
        case 11:
          line.to   = top;
          line.from = right;
          break;
        case 5:
          line.to   = top;
          line.from = left;
          canvas_draw_line(canvas, line);

          line.to   = bottom;
          line.from = right;
          break;
        case 6:
        case 9:
          line.to   = bottom;
          line.from = top;
          break;
        case 7:
        case 8:
          line.to   = left;
          line.from = top;
          break;
        case 10:
          line.to   = bottom;
          line.from = left;
          canvas_draw_line(canvas, line);

          line.to   = top;
          line.from = right;
          break;
      }

      canvas_draw_line(canvas, line);
    }
  }

  canvas_present(canvas);
}

#if !defined(__wasm__) || !defined(__wasm32__)
int main(void) {
  int result = 0;
  SDL_Window*   window      = null;
  SDL_Renderer* renderer    = null;
  SDL_Texture*  framebuffer = null;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("Marching squares",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            0);
  if (!window) {
    SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
    return_defer(1);
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
    return_defer(1);
  }

  framebuffer = SDL_CreateTexture(renderer,
                                  SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  CANVAS_WIDTH, CANVAS_HEIGHT);
  if (!framebuffer) {
    SDL_Log("SDL_CreateTexture Error: %s", SDL_GetError());
    return_defer(1);
  }

  Canvas* canvas = create_canvas(CANVAS_WIDTH, CANVAS_HEIGHT);
  init_scene(canvas);

  bool is_running = true;
  SDL_Event event;
  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) is_running = false;
    }

    canvas_update(canvas, 1.0/60.0);
    canvas_render(canvas);

    SDL_RenderClear(renderer);
    void *pixels;
    int pitch;
    if (SDL_LockTexture(framebuffer, null, &pixels, &pitch) != 0) {
      SDL_Log("SDL_LockTexture Error: %s", SDL_GetError());
    } else {
      memcpy(pixels, canvas->color_buffer, canvas->width*canvas->height*4);
      SDL_UnlockTexture(framebuffer);
    }

    SDL_Rect rect = {
      .w = canvas->width,
      .h = canvas->height,
    };
    SDL_RenderCopy(renderer, framebuffer, &rect, null);
    SDL_RenderPresent(renderer);

    SDL_Delay(16);
  }

defer:
  if (framebuffer) SDL_DestroyTexture(framebuffer);
  if (renderer)    SDL_DestroyRenderer(renderer);
  if (window)      SDL_DestroyWindow(window);
  SDL_Quit();

  return result;
}
#endif