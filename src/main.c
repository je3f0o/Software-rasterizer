/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.c
 * Created at  : 2025-05-24
 * Updated at  : 2025-06-13
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

#ifndef M_PIf
  #define M_PIf	3.14159265358979323846f	/* pi */
#endif
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800
#define CANVAS_WIDTH  WINDOW_WIDTH
#define CANVAS_HEIGHT WINDOW_HEIGHT

#define GRAY   0xFF181818
#define RED    0xFF0000FF
#define GREEN  0xFF00FF00
#define BLUE   0xFFFF0000

// 3D scene buffers
Vertex3D vertices[] = {
  { {0, 0, 0} , {255,   0,   0, 255} },
  { {0, 0, 0} , {  0, 255,   0, 255} },
  { {0, 0, 0} , {  0,   0, 255, 255} },

  { {0, 0, 0} , {255,   0,   0, 255} },
  { {0, 0, 0} , {  0, 255,   0, 255} },
  { {0, 0, 0} , {  0,   0, 255, 255} },
};
Vertex3D dest_vertices[6] = {0};

// 2D scene buffers
Vertex2D vertices_2d[] = {
  { {0, 0} , {255,   0,   0, 255} },
  { {0, 0} , {  0, 255,   0, 255} },
  { {0, 0} , {  0,   0, 255, 255} },
};
Vertex2D dest_vertices_2d[3] = {0};

//void render(const Canvas* canvas) {
//  assert(canvas != null);
//  stbi_write_png("image3.png", canvas->width, canvas->height, 4, canvas->data, 0);
//}

void vec2_rotate_at(vec2i* dest, vec2i* p, vec2i pivot, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  float px = p->x - pivot.x;
  float py = p->y - pivot.y;
  float x = c*px - s*py;
  float y = s*px + c*py;
  dest->x = (int)x + pivot.x;
  dest->y = (int)y + pivot.y;
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

void init_scene_3d(Canvas* canvas) {
  UNUSED(canvas);
  float z = 1.25;

  vertices[0].position = (vec3) {-0.5, -0.5, z};
  vertices[1].position = (vec3) {   0,  0.5, z};
  vertices[2].position = (vec3) {+0.5, -0.5, z};

  vec3 pivot = {0, 0, z};
  for (size_t i = 0; i < 3; ++i) {
    vec3_rotate_y_at(&vertices[i+3].position, &vertices[i].position, pivot, radians(-90));
  }

  memcpy(dest_vertices, vertices, sizeof(vertices));
}

void canvas_update_3d(Canvas* canvas, double dt) {
  UNUSED(canvas);
  static float angle = 0;
  angle += radians(2*dt);
  vec3 center = {0, 0, vertices[0].position.z};

  for (u32 i = 0; i < ARRAY_LENGTH(vertices); ++i) {
    vec3_rotate_y_at(&dest_vertices[i].position, &vertices[i].position, center, angle);
  }
}

void canvas_render_3d(Canvas* canvas) {
  canvas_clear(canvas, GRAY);

  canvas_fill_triangle_3d(canvas, dest_vertices);
  canvas_fill_triangle_3d(canvas, &dest_vertices[3]);
}

void init_scene_2d(Canvas* canvas) {
  vertices_2d[0].position = (vec2i) {canvas->width*0.5, canvas->height*0.1};
  vertices_2d[1].position = (vec2i) {canvas->width*0.9, canvas->height*0.9};
  vertices_2d[2].position = (vec2i) {canvas->width*0.1, canvas->height*0.9};

  memcpy(dest_vertices_2d, vertices_2d, sizeof(vertices_2d));
}

void canvas_update_2d(Canvas* canvas, double dt) {
  static float angle = 0;
  angle += radians(2*dt);
  vec2i center = {canvas->width*0.5, canvas->height*0.5};
  for (u32 i = 0; i < ARRAY_LENGTH(vertices_2d); ++i) {
    vec2_rotate_at(&dest_vertices_2d[i].position, &vertices_2d[i].position, center, angle);
  }
}

void canvas_render_2d(Canvas* canvas) {
  // clear background
  canvas_clear(canvas, GRAY);
  canvas_fill_triangle_2d(canvas, dest_vertices_2d);
}

int main(void) {
  int result = 0;
  SDL_Window*   window      = null;
  SDL_Renderer* renderer    = null;
  SDL_Texture*  framebuffer = null;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init Error: %s", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("SDL2 Hello World",
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
  init_scene_2d(canvas);
  init_scene_3d(canvas);

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
    if (SDL_LockTexture(framebuffer, null, &pixels, &pitch) != 0) {
      SDL_Log("SDL_LockTexture Error: %s", SDL_GetError());
    } else {
      canvas_update_3d(canvas, 0.16);
      canvas_render_3d(canvas);
      memcpy(pixels, canvas->color_buffer, canvas->width*canvas->height*4);

      SDL_UnlockTexture(framebuffer);
    }

    SDL_RenderCopy(renderer, framebuffer, null, null);
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