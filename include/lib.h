#ifndef __JEEFO_LIB_H__
#define __JEEFO_LIB_H__

#include <stdint.h>

#if defined(__wasm__) || defined(__wasm32__)
#define NULL 0
#else
#include <stddef.h>
#endif

typedef int        i32;
typedef uint32_t   u32;
typedef uint8_t    u8;

typedef struct {
  u32   width;
  u32   height;
  i32   data[];
} Canvas;

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

typedef struct {
  int x, y;
} vec2;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

typedef struct {
  vec2  point;
  Color color;
} Vertex;

typedef struct {
  int x, y;
  int width;
  int height;
} Rect;

Canvas*  create_canvas(u32 width, u32 height);
void     destroy_canvas(Canvas* canvas);

void   canvas_fill_triangle(Canvas* canvas, Vertex triangle[3]);
void   canvas_fill_rect(Canvas* canvas, Rect rect, int color);
void   canvas_clear(Canvas* canvas, int color);

u32   canvas_width(Canvas* canvas);
u32   canvas_height(Canvas* canvas);

#endif