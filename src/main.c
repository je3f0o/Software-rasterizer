/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.c
 * Created at  : 2025-05-24
 * Updated at  : 2025-06-08
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include <stdbool.h>
#include "lib.h"

#if defined(__wasm__) || defined(__wasm32__)
void assert(bool);
int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
#else
#include <math.h>
#include <assert.h>
#include "stb_image_write.h"
#endif

#define GRAY   0xFF181818
#define RED    0xFF0000FF
#define GREEN  0xFF00FF00
#define BLUE   0xFFFF0000

void render(const Canvas* canvas) {
  assert(canvas != NULL);
  stbi_write_png("image3.png", canvas->width, canvas->height, 4, canvas->data, 0);
}

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

void vec2_rotate_at(vec2* p, vec2 pivot, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  float px = p->x - pivot.x;
  float py = p->y - pivot.y;
  float x = c*px - s*py;
  float y = s*px + c*py;
  p->x = x + pivot.x;
  p->y = y + pivot.y;
}

float radians(float degrees) {
  return degrees * M_PIf/180.0;
}

void canvas_draw_scene(Canvas* canvas) {
  // clear background
  canvas_clear(canvas, GRAY);

  //vec2 triangle1[3] = { {20, 30}, {80, 70}, {20, 70} };
  //vec2 triangle2[3] = { {20, 30}, {80, 30}, {80, 70} };
  //vec2 triangle3[3] = { {50, 10}, {80, 30}, {20, 30}};

  //rasterize_triangle(triangle2, 0xFFFF0000);
  //rasterize_triangle(triangle3, 0xFF0000FF);

  //Vertex triangle[3] = {
  //  { {(float)WIDTH * 0.5 , (float)HEIGHT * 0.1} , {255 , 0   , 0} }   ,
  //  { {(float)WIDTH * 0.1 , (float)HEIGHT * 0.9} , {  0 , 255 , 0} }   ,
  //  { {(float)WIDTH * 0.9 , (float)HEIGHT * 0.9} , {  0 , 0   , 255} } ,
  //};
  Vertex triangle[] = {
    { {(float)canvas->width * 0.5 , (float)canvas->height * 0.1} , {255 , 0 , 0} },
    { {(float)canvas->width * 0.9 , (float)canvas->height * 0.5} , {255 , 0 , 0} },
    { {(float)canvas->width * 0.1 , (float)canvas->height * 0.5} , {255 , 0 , 0} },

    { {(float)canvas->width * 0.1 , (float)canvas->height * 0.5} , {0, 0 , 255} },
    { {(float)canvas->width * 0.9 , (float)canvas->height * 0.5} , {0, 0 , 255} },
    { {(float)canvas->width * 0.5 , (float)canvas->height * 0.9} , {0, 0 , 255} },
  };

  vec2 center = {canvas->width*0.5, canvas->height*0.5};

  for (size_t i = 0; i < ARRAY_LENGTH(triangle); ++i) {
    //vec2_rotate(&triangle[i].point, radians(10));
    vec2_rotate_at(&triangle[i].point, center, radians(30));
  }

  //canvas_fill_triangle(canvas, triangle);
  canvas_fill_triangle(canvas, &triangle[3]);

  canvas_fill_rect(canvas, (Rect) {
    .x      = 50,
    .y      = 50,
    .width  = 100,
    .height = 100,
  }, RED);

  // TODO: add more render targets
  render(canvas);
}

int main(void) {
  Canvas* canvas = create_canvas(50, 50);
  canvas_draw_scene(canvas);
  destroy_canvas(canvas);
  return 0;
}