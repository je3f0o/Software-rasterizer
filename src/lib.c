/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : lib.c
 * Created at  : 2025-06-04
 * Updated at  : 2025-06-08
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include "lib.h"
#include <stdbool.h>

#if defined(__wasm__) || defined(__wasm32__)
void* malloc(unsigned long size);
void  free(void* ptr);
int   abs(int j);
#else
//#include <stdio.h>
#include <stdlib.h>
#endif

//static int pixels_data[800*600];

Canvas* create_canvas(u32 width, u32 height) {
  Canvas* canvas = malloc(sizeof(Canvas) + sizeof(int) * width*height);
  canvas->width  = width;
  canvas->height = height;
  return canvas;
}

void destroy_canvas(Canvas* canvas) {
  free(canvas);
}

static int cross_edge(vec2 a, vec2 b, vec2 p) {
  vec2 ab = {b.x - a.x, b.y - a.y};
  vec2 ap = {p.x - a.x, p.y - a.y};

  return ab.x * ap.y - ab.y * ap.x;
}

static inline void canvas_put_pixel(Canvas* canvas, u32 x, u32 y, int color) {
  canvas->data[y * canvas->width + x] = color;
}

//static float lerp(float v0, float v1, float t) {
//  return v0 + t * (v1 - v0);
//}

void canvas_fill_triangle(Canvas* canvas, Vertex triangle[3]) {
  vec2 v0 = triangle[0].point;
  vec2 v1 = triangle[1].point;
  vec2 v2 = triangle[2].point;

  float area = cross_edge(v0, v1, v2);

  int min_x = MIN(MIN(v0.x, v1.x), v2.x);
  int min_y = MIN(MIN(v0.y, v1.y), v2.y);
  int max_x = MAX(MAX(v0.x, v1.x), v2.x);
  int max_y = MAX(MAX(v0.y, v1.y), v2.y);

  for (int y = min_y; y <= max_y; ++y) {
    for (int x = min_x; x <= max_x; ++x) {
      vec2 p = {x, y};
      int w0 = cross_edge(v1, v2, p);
      int w1 = cross_edge(v2, v0, p);
      int w2 = cross_edge(v0, v1, p);

      if ((w0 | w1 | w2) > 0) {
        float r0 = w0 / area;
        float r1 = w1 / area;
        float r2 = w2 / area;

        int r = r0 * triangle[0].color.r + r1 * triangle[1].color.r + r2 * triangle[2].color.r;
        int g = r0 * triangle[0].color.g + r1 * triangle[1].color.g + r2 * triangle[2].color.g;
        int b = r0 * triangle[0].color.b + r1 * triangle[1].color.b + r2 * triangle[2].color.b;
        int a = 0xFF;

        // Clamp to 0–255
        r = CLAMP(r, 0, 255);
        g = CLAMP(g, 0, 255);
        b = CLAMP(b, 0, 255);

        int color = 0;
        color = (color | (a & 0xFF)) << 8;
        color = (color | (b & 0xFF)) << 8;
        color = (color | (g & 0xFF)) << 8;
        color = (color | (r & 0xFF));

        canvas_put_pixel(canvas, x, y, color);
      }
    }
  }
}

void canvas_fill_rect(Canvas* canvas, Rect rect, int color) {
  int min_x = MIN(rect.x, rect.x + rect.width);
  int min_y = MIN(rect.y, rect.y + rect.height);
  int max_x = MAX(rect.x, rect.x + rect.width);
  int max_y = MAX(rect.y, rect.y + rect.height);

  bool is_rect_outside = (
    max_x < 0 ||
    max_y < 0 ||
    min_x > (i32)canvas->width ||
    min_y > (i32)canvas->height
  );
  if (is_rect_outside) return;

  int offset_x = CLAMP(min_x, 0, (i32)canvas->width);
  int offset_y = CLAMP(min_y, 0, (i32)canvas->height);

  int width  = max_x - min_x;
  int height = max_y - min_y;
  if (min_x < 0) width  += min_x;
  if (min_y < 0) height += min_y;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      canvas_put_pixel(canvas, x+offset_x, y+offset_y, color);
    }
  }
}

void canvas_clear(Canvas* canvas, int color) {
  for (u32 y = 0; y < canvas->height; ++y) {
    for (u32 x = 0; x < canvas->width; ++x) {
      canvas_put_pixel(canvas, x, y, color);
    }
  }
}

u32 canvas_width(Canvas* canvas)  { return canvas->width; }
u32 canvas_height(Canvas* canvas) { return canvas->height; }