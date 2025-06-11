/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.c
 * Created at  : 2025-05-24
 * Updated at  : 2025-06-11
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "lib.h"

#if defined(__wasm__) || defined(__wasm32__)
void  assert(bool);
int   stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);

float cosf(float);
float sinf(float);
#else
#include <math.h>
#include <assert.h>
#include "stb_image_write.h"
#endif

#define GRAY   0xFF181818
#define RED    0xFF0000FF
#define GREEN  0xFF00FF00
#define BLUE   0xFFFF0000

Vertex3D vertices[] = {
  { {0, 0, 0} , {255 , 0 , 0, 255} },
  { {0, 0, 0} , {255 , 0 , 0, 255} },
  { {0, 0, 0} , {255 , 0 , 0, 255} },

  { {0, 0, 0} , {255, 0 , 255, 255} },
  { {0, 0, 0} , {255, 0 , 255, 255} },
  { {0, 0, 0} , {255, 0 , 255, 255} },
};
Vertex3D dest_vertices[6] = {0};

//void render(const Canvas* canvas) {
//  assert(canvas != NULL);
//  stbi_write_png("image3.png", canvas->width, canvas->height, 4, canvas->data, 0);
//}

#ifndef M_PIf
  #define M_PIf	3.14159265358979323846f	/* pi */
#endif
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

void vec2_rotate(vec2* p, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  float x = c*p->x - s*p->y;
  float y = s*p->x + c*p->y;
  p->x = x;
  p->y = y;
}

void vec2_rotate_at(vec2* dest, vec2* p, vec2 pivot, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  float px = (float)p->x - (float)pivot.x;
  float py = (float)p->y - (float)pivot.y;
  float x = c*px - s*py;
  float y = s*px + c*py;
  dest->x = (int)roundf(x) + pivot.x;
  dest->y = (int)roundf(y) + pivot.y;
}

void vec3_rotate_y_at(vec3* dest, vec3* p, vec3 pivot, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);

  float px = p->x - pivot.x;
  float pz = p->z - pivot.z;

  // Rotate around Y-axis
  float x =  c * px + s * pz;
  float z = -s * px + c * pz;

  // Translate back to world space
  dest->x = x + pivot.x;
  dest->y = p->y;
  dest->z = z + pivot.z;
}

float radians(float degrees) {
  return degrees * M_PIf/180.0;
}

void init_scene(Canvas* canvas) {
  (void)canvas;
  float z = 1.25;

  vertices[0].position = (vec3) {-0.5, -0.5, z};
  vertices[0].color    = (Color) {255, 0, 0, 255};
  vertices[3].color    = (Color) {255, 0, 0, 255};
  vertices[1].position = (vec3) {   0,  0.5, z};
  vertices[1].color    = (Color) {0, 255, 0, 255};
  vertices[4].color    = (Color) {0, 255, 0, 255};
  vertices[2].position = (vec3) {+0.5, -0.5, z};
  vertices[2].color    = (Color) {0, 0, 255, 255};
  vertices[5].color    = (Color) {0, 0, 255, 255};

  vec3 pivot = {0, 0, z};
  for (size_t i = 0; i < 3; ++i) {
    vec3_rotate_y_at(&vertices[i+3].position, &vertices[i].position, pivot, radians(-90));
  }

  memcpy(dest_vertices, vertices, sizeof(vertices));
}

void canvas_update(Canvas* canvas, double dt) {
  (void)dt;
  (void)canvas;
  static float angle = 0;
  angle += radians(2*dt);
  vec3 center = {0, 0, vertices[0].position.z};

  for (u32 i = 0; i < ARRAY_LENGTH(vertices); ++i) {
    vec3_rotate_y_at(&dest_vertices[i].position, &vertices[i].position, center, angle);
  }
}

bool draw_2nd_triangle = true;

void canvas_render(Canvas* canvas) {
  // clear background
  canvas_clear(canvas, GRAY);

  canvas_fill_triangle_3d(canvas, dest_vertices);
  canvas_fill_triangle_3d(canvas, &dest_vertices[3]);

  //canvas_fill_rect(canvas, (Rect) {
  //  .x      = 49,
  //  .y      = 0,
  //  .width  = 100,
  //  .height = 100,
  //}, RED);

  // TODO: add more render targets
  //render(canvas);
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800
#define CANVAS_WIDTH  800
#define CANVAS_HEIGHT 800

int main(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("SDL2 Hello World",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        0);
  if (!window) {
    SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Create a texture that we will use as our framebuffer
  SDL_Texture *framebuffer = SDL_CreateTexture(renderer,
                                               SDL_PIXELFORMAT_RGBA32,
                                               SDL_TEXTUREACCESS_STREAMING,
                                               CANVAS_WIDTH, CANVAS_HEIGHT);
  if (!framebuffer) {
    SDL_Log("SDL_CreateTexture Error: %s", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  Canvas* canvas = create_canvas(CANVAS_WIDTH, CANVAS_HEIGHT);
  init_scene(canvas);

  bool is_running = true;
  SDL_Event event;
  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) is_running = false;
      //else if (event.type == SDL_MOUSEBUTTONDOWN) {
      //  if (event.button.button == SDL_BUTTON_LEFT) {
      //    draw_2nd_triangle = !draw_2nd_triangle;
      //  }
      //}
    }

    SDL_RenderClear(renderer);

    void *pixels;
    int pitch;
    if (SDL_LockTexture(framebuffer, NULL, &pixels, &pitch) != 0) {
      SDL_Log("SDL_LockTexture Error: %s", SDL_GetError());
    } else {
      canvas_update(canvas, 0.16);
      canvas_render(canvas);
      memcpy(pixels, canvas->color_buffer, canvas->width*canvas->height*4);

      SDL_UnlockTexture(framebuffer);
    }

    SDL_Rect dest_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, framebuffer, NULL, &dest_rect);
    SDL_RenderPresent(renderer);

    SDL_Delay(16);
  }

  SDL_DestroyTexture(framebuffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}