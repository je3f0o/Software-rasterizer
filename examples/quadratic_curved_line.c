/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : quadratic_curved_line.c
 * Created at  : 2025-07-11
 * Updated at  : 2025-11-08
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

static Point points[3]     = {0};
static int   dragged_index = -1;
static Color line_color = (Color){.raw = 0xFF555555};

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

static void bezier_curve(Canvas* canvas) {
  Circle circle = {.radius = 7};
  int STEP = 100;
  for (int i = 0; i < STEP; ++i) {
    float t = (float)i/STEP;
    float x1 = lerp(points[0].circle.x, points[1].circle.x, t);
    float y1 = lerp(points[0].circle.y, points[1].circle.y, t);
    float x2 = lerp(points[1].circle.x, points[2].circle.x, t);
    float y2 = lerp(points[1].circle.y, points[2].circle.y, t);

    canvas_draw_line(canvas, {
      .from        = {x1, y1},
      .to          = {x2, y2},
      .color       = line_color,
      .antialiased = true,
    });

    circle.x = x1;
    circle.y = y1;
    canvas_fill_circle(canvas, circle, line_color);
    circle.x = x2;
    circle.y = y2;
    canvas_fill_circle(canvas, circle, line_color);
  }
}

void init_scene(Canvas* canvas) {
  UNUSED(canvas);
  int radius = 10;
  points[0].color         = GREEN;
  points[0].circle.x      = canvas->width  * 0.1;
  points[0].circle.y      = canvas->height * 0.9;
  points[0].circle.radius = radius;

  points[1].color         = BLUE;
  points[1].circle.x      = canvas->width  * 0.5;
  points[1].circle.y      = canvas->height * 0.1;
  points[1].circle.radius = radius;

  points[2].color         = RED;
  points[2].circle.x      = canvas->width  * 0.9;
  points[2].circle.y      = canvas->height * 0.9;
  points[2].circle.radius = radius;
}

void canvas_update(Canvas* canvas, double dt) {
  UNUSED(dt);
  UNUSED(canvas);
}

void canvas_render(Canvas* canvas) {
  UNUSED(bezier_curve);
  canvas_clear(canvas, GRAY);

  for (size_t i = 0; i < ARRAY_LENGTH(points) - 1; ++i) {
    canvas_draw_line(canvas, {
      .from        = {points[i+0].circle.x, points[i+0].circle.y},
      .to          = {points[i+1].circle.x, points[i+1].circle.y},
      .color       = (Color){.raw = 0xFF555555},
      .antialiased = true,
    });
  }

  canvas_draw_quadratic_curved_line(canvas, {
    .v0          = {points[0].circle.x, points[0].circle.y},
    .v1          = {points[1].circle.x, points[1].circle.y},
    .v2          = {points[2].circle.x, points[2].circle.y},
    .color       = PURPLE,
    .antialiased = true,
  });

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

  window = SDL_CreateWindow("Interactive quadratic curved line",
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