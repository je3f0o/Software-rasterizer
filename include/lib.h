#ifndef __JEEFO_LIB_H__
#define __JEEFO_LIB_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__wasm__) || defined(__wasm32__)
void* malloc(unsigned long size);
void  free(void* ptr);
void* memcpy(void*, const void*, size_t __n);
void* memset(void *__s, int __c, size_t __n);
void  assert(bool);
int   abs(int j);
float cosf(float);
float sinf(float);
#else
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifndef NULL
  #define NULL 0
#endif
#define null NULL

#ifndef AA
  #define AA 2
#endif

#ifndef M_PIf
  #define M_PIf	3.14159265358979323846f
#endif
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#define INLINE static inline

#define UNUSED(x) ((void)x)

#define UNIMPLEMENTED(...) do { \
  fprintf(stderr, "%s:%d: %s is not implemented yet\n", \
          __FILE__, __LINE__, __func__); \
  abort(); \
} while(0)

#define return_defer(value) do { \
  result = value;                \
  goto defer;                    \
} while(0)

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#define GRAY   (Color){.raw = 0xFF181818}
#define RED    (Color){.raw = 0xFF0000FF}
#define GREEN  (Color){.raw = 0xFF00FF00}
#define BLUE   (Color){.raw = 0xFFFF0000}
#define PURPLE (Color){.raw = 0xFFFF00FF}

typedef int32_t    i32;
typedef uint32_t   u32;
typedef uint8_t    u8;
typedef float      f32;
typedef double     f64;

typedef struct {
  i32   width;
  i32   height;
  i32*  color_buffer;
  f32*  depth_buffer;
} Canvas;

typedef struct {
  i32 x, y;
} vec2i;

typedef struct {
  f32 x, y;
} vec2;

typedef struct {
  f32 x, y, z;
} vec3;

typedef union {
  struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
  } rgba;
  i32 raw;
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
  i32 x, y;
  i32 width;
  i32 height;
} Rect;

typedef struct {
  i32 x, y;
  u32 radius;
} Circle;

Canvas*  create_canvas(u32 width, u32 height);
void     destroy_canvas(Canvas* canvas);

Color blend_color(Color bg, Color fg);

u32   canvas_width(Canvas* canvas);
u32   canvas_height(Canvas* canvas);
void  canvas_clear(Canvas* canvas, Color color);

void  canvas_fill_triangle_2d(Canvas* canvas, Vertex2D triangle[3]);
void  canvas_fill_triangle_3d(Canvas* canvas, Vertex3D triangle[3]);
void  canvas_fill_rect(Canvas* canvas, Rect rect, Color color);
void  canvas_fill_circle(Canvas* canvas, Circle circle, Color color);
void  canvas_stroke_circle(Canvas* canvas, Circle circle, Color color);

typedef struct {
  vec2i p0, p1;
  Color color;
  bool  antialiased;
} LineOptions;

void  _canvas_draw_line(Canvas* canvas, LineOptions options);
#define canvas_draw_line(canvas, ...) \
  _canvas_draw_line(canvas, (LineOptions)__VA_ARGS__)

void  canvas_draw_lines(Canvas* canvas, LineOptions* lines, size_t count);

vec2  project_3d_to_2d(vec3 p);
vec2i project_2d_to_screen(Canvas* canvas, vec2 p);

#endif