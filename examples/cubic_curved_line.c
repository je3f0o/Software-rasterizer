/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : cubic_curved_line.c
 * Created at  : 2025-11-08
 * Updated at  : 2025-11-24
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include "lib.h"
#include <math.h>

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


void canvas_present(Canvas* canvas) {
  assert(canvas != null);
#if defined(__wasm__) || defined(__wasm32__) || defined(PLATFORM_IMAGE)
  stbi_write_png("image.png", canvas->width, canvas->height, 4, canvas->color_buffer, 0);
#elif defined(PLATFORM_SDL)

#endif
}

static float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

typedef struct {
  Color  color;
  Circle circle;
} Point;

static Point points[4]     = {0};
static float t             = 0;
static int   dragged_index = -1;
static Color line_color    = (Color){.raw = 0xFF555555};

static int get_dragged_point_index(int x, int y) {
  for (size_t i = 0; i < ARRAY_LENGTH(points); ++i) {
    int dx = x - points[i].circle.x;
    int dy = y - points[i].circle.y;
    int r  = points[i].circle.radius;
    if (dx*dx+dy*dy < r*r) {
      return i;
    }
  }
  return -1;
}

void init_scene(Canvas* canvas) {
  UNUSED(canvas);
  int radius = 10;
  points[0].color         = GREEN;
  points[0].circle.x      = canvas->width  * 0.1;
  points[0].circle.y      = canvas->height * 0.6;
  points[0].circle.radius = radius;

  points[1].color         = BLUE;
  points[1].circle.x      = canvas->width  * 0.3;
  points[1].circle.y      = canvas->height * 0.2;
  points[1].circle.radius = radius;

  points[2].color         = BLUE;
  points[2].circle.x      = canvas->width  * 0.7;
  points[2].circle.y      = canvas->height * 0.8;
  points[2].circle.radius = radius;

  points[3].color         = RED;
  points[3].circle.x      = canvas->width  * 0.9;
  points[3].circle.y      = canvas->height * 0.4;
  points[3].circle.radius = radius;
}

void canvas_update(Canvas* canvas, double dt) {
  UNUSED(dt);
  UNUSED(canvas);
}

void canvas_render(Canvas* canvas) {
  canvas_clear(canvas, GRAY);

  for (size_t i = 0; i < ARRAY_LENGTH(points) - 1; ++i) {
    canvas_draw_line(canvas, {
      .from        = {points[i+0].circle.x, points[i+0].circle.y},
      .to          = {points[i+1].circle.x, points[i+1].circle.y},
      .color       = line_color,
      .antialiased = true,
    });
  }

  canvas_draw_cubic_curved_line(canvas, {
    .p0          = {points[0].circle.x, points[0].circle.y},
    .p1          = {points[1].circle.x, points[1].circle.y},
    .p2          = {points[2].circle.x, points[2].circle.y},
    .p3          = {points[3].circle.x, points[3].circle.y},
    .color       = PURPLE,
    .antialiased = true,
  });

  if (t > 0.0 && t < 1.0) {
    float x1 = lerp(points[0].circle.x, points[1].circle.x, t);
    float y1 = lerp(points[0].circle.y, points[1].circle.y, t);
    float x2 = lerp(points[1].circle.x, points[2].circle.x, t);
    float y2 = lerp(points[1].circle.y, points[2].circle.y, t);
    float x3 = lerp(points[2].circle.x, points[3].circle.x, t);
    float y3 = lerp(points[2].circle.y, points[3].circle.y, t);

    float x4 = lerp(x1, x2, t);
    float y4 = lerp(y1, y2, t);
    float x5 = lerp(x2, x3, t);
    float y5 = lerp(y2, y3, t);

    float final_x = lerp(x4, x5, t);
    float final_y = lerp(y4, y5, t);

    canvas_draw_line(canvas, {
      .from        = {x1, y1},
      .to          = {x2, y2},
      .color       = CYAN,
      .antialiased = true,
    });
    canvas_draw_line(canvas, {
      .from        = {x2, y2},
      .to          = {x3, y3},
      .color       = CYAN,
      .antialiased = true,
    });
    canvas_draw_line(canvas, {
      .from        = {x4, y4},
      .to          = {x5, y5},
      .color       = GREEN,
      .antialiased = true,
    });

    Circle circle = {.radius = 7};
    circle.x = x1;
    circle.y = y1;
    canvas_fill_circle(canvas, circle, line_color);
    circle.x = x2;
    circle.y = y2;
    canvas_fill_circle(canvas, circle, line_color);
    circle.x = x3;
    circle.y = y3;
    canvas_fill_circle(canvas, circle, line_color);

    circle.x = x4;
    circle.y = y4;
    canvas_fill_circle(canvas, circle, CYAN);
    circle.x = x5;
    circle.y = y5;
    canvas_fill_circle(canvas, circle, CYAN);

    circle.x = final_x;
    circle.y = final_y;
    canvas_fill_circle(canvas, circle, GREEN);
  }

  for (size_t i = 0; i < ARRAY_LENGTH(points); ++i) {
    canvas_fill_circle(canvas, points[i].circle, points[i].color);
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

  window = SDL_CreateWindow("Interactive cubic curved line",
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

      if (dragged_index >= 0) {
        points[dragged_index].circle.x = event.motion.x;
        points[dragged_index].circle.y = event.motion.y;
      }

      if (event.key.state == SDL_PRESSED) {
        switch (event.key.keysym.sym) {
          case SDLK_LEFT:
            t = fmaxf(0, t - 0.05);
            break;
          case SDLK_RIGHT:
            t = fminf(1, t + 0.05);
            break;
        }
      }

      switch (event.button.type) {
        case SDL_MOUSEBUTTONDOWN: {
          int x = event.button.x;
          int y = event.button.y;
          dragged_index = get_dragged_point_index(x, y);
        } break;
        case SDL_MOUSEBUTTONUP:
          dragged_index = -1;
          break;
      }
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