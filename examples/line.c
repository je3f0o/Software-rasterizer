/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : line.c
 * Created at  : 2025-07-11
 * Updated at  : 2025-08-26
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
#define CANVAS_WIDTH  80
#define CANVAS_HEIGHT 80

void canvas_present(Canvas* canvas) {
  assert(canvas != null);
#if defined(__wasm__) || defined(__wasm32__) || defined(PLATFORM_IMAGE)
  stbi_write_png("image.png", canvas->width, canvas->height, 4, canvas->color_buffer, 0);
#elif defined(PLATFORM_SDL)
#endif
}

void init_scene(Canvas* canvas) {
  UNUSED(canvas);
}

void canvas_update(Canvas* canvas, double dt) {
  UNUSED(dt);
  UNUSED(canvas);
}

void canvas_render(Canvas* canvas) {
  canvas_clear(canvas, GRAY);

  LineOptions lines[] = {
    // X axis
    (LineOptions){
      .p0 = {
        .y = canvas->height / 2,
      },
      .p1 = {
        .x = canvas->width,
        .y = canvas->height / 2,
      },
      .color = RED,
    },

    // Y axis
    (LineOptions){
      .p0 = {
        .x = canvas->width / 2,
      },
      .p1 = {
        .x = canvas->width / 2,
        .y = canvas->height / 5 * 4,
      },
      .color = BLUE,
    },

    // No steep lines
    (LineOptions){
      .p0 = {
        .x = canvas->width,
        .y = canvas->height / 2 - 10,
      },
      .p1 = {
        .y = canvas->height - 10,
      },
      .color = GREEN,
    },

    (LineOptions){
      .p0 = {
        .x = canvas->width,
        .y = canvas->height / 2,
      },
      .p1 = {
        .y = canvas->height,
      },
      .color       = PURPLE,
      .antialiased = true,
    },

    // Steep lines
    (LineOptions){
      .p0 = {
        .x = canvas->width / 5,
      },
      .p1 = {
        .x = canvas->width / 2,
        .y = canvas->height,
      },
      .color = GREEN,
    },

    (LineOptions){
      .p0 = {
        .x = canvas->width / 5 + 10,
      },
      .p1 = {
        .x = canvas->width / 2 + 10,
        .y = canvas->height,
      },
      .color       = PURPLE,
      .antialiased = true,
    },
  };

  canvas_draw_lines(canvas, lines, ARRAY_LENGTH(lines));

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

  window = SDL_CreateWindow("Anti aliased on/off lines",
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