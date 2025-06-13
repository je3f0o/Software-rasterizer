#ifndef __JEEFO_LIB_H__
#define __JEEFO_LIB_H__

#include <stddef.h>
#include <stdint.h>

#if defined(__wasm__) || defined(__wasm32__)
void* malloc(unsigned long size);
void  free(void* ptr);
int   abs(int j);
#endif

#ifndef NULL
  #define NULL 0
#endif
#define null NULL

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#define UNUSED(x) ((void)x)

#define return_defer(value) do { \
  result = value;                \
  goto defer;                    \
} while(0)

typedef int        i32;
typedef uint32_t   u32;
typedef uint8_t    u8;

typedef struct {
  i32     width;
  i32     height;
  i32*    color_buffer;
  float*  depth_buffer;
} Canvas;

typedef struct {
  int x, y;
} vec2i;

typedef struct {
  float x, y;
} vec2;

typedef struct {
  float x, y, z;
} vec3;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} Color;

typedef struct {
  vec2i  position;
  Color  color;
} Vertex2D;

typedef struct {
  vec3  position;
  Color color;
} Vertex3D;

typedef struct {
  int x, y;
  int width;
  int height;
} Rect;

Canvas*  create_canvas(u32 width, u32 height);
void     destroy_canvas(Canvas* canvas);

void   canvas_fill_triangle_2d(Canvas* canvas, Vertex2D triangle[3]);
void   canvas_fill_triangle_3d(Canvas* canvas, Vertex3D triangle[3]);
void   canvas_fill_rect(Canvas* canvas, Rect rect, int color);
void   canvas_clear(Canvas* canvas, int color);

u32   canvas_width(Canvas* canvas);
u32   canvas_height(Canvas* canvas);

vec2  project_3d_to_2d(vec3 p);
vec2i project_2d_to_screen(Canvas* canvas, vec2 p);

#endif