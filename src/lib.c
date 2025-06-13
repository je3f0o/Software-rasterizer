/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : lib.c
 * Created at  : 2025-06-04
 * Updated at  : 2025-06-13
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include "lib.h"

Canvas* create_canvas(u32 width, u32 height) {
  size_t color_buffer_size = sizeof(u32)   * width * height;
  size_t depth_buffer_size = sizeof(float) * width * height;
  size_t buffer_size = sizeof(Canvas) + color_buffer_size + depth_buffer_size;

  u8* buffer = malloc(buffer_size);
  Canvas* canvas       = (Canvas*)buffer;
  canvas->width        = width;
  canvas->height       = height;
  canvas->color_buffer = (i32*)&buffer[sizeof(Canvas)];
  canvas->depth_buffer = (float*)&buffer[sizeof(Canvas) + color_buffer_size];

  return canvas;
}

void destroy_canvas(Canvas* canvas) {
  free(canvas);
}

static int cross_edge(vec2i a, vec2i b, vec2i p) {
  vec2i ab = {b.x - a.x, b.y - a.y};
  vec2i ap = {p.x - a.x, p.y - a.y};

  return ab.x * ap.y - ab.y * ap.x;
}

static inline void canvas_put_pixel(Canvas* canvas, u32 x, u32 y, int color) {
  canvas->color_buffer[y * canvas->width + x] = color;
}

//static float lerp(float v0, float v1, float t) {
//  return v0 + t * (v1 - v0);
//}

bool is_top_left_edge(vec2i v0, vec2i v1) {
  vec2i edge = {v1.x - v0.x, v1.y - v0.y};
  bool is_top_edge  = edge.y == 0 && edge.x > 0;
  bool is_left_edge = edge.y < 0;
  return is_top_edge || is_left_edge;
}

void canvas_fill_triangle_2d(Canvas* canvas, Vertex2D triangle[3]) {
  vec2i v0 = triangle[0].position;
  vec2i v1 = triangle[1].position;
  vec2i v2 = triangle[2].position;

  float area = cross_edge(v0, v1, v2);

  int min_x = MIN(MIN(v0.x, v1.x), v2.x);
  int min_y = MIN(MIN(v0.y, v1.y), v2.y);
  int max_x = MAX(MAX(v0.x, v1.x), v2.x);
  int max_y = MAX(MAX(v0.y, v1.y), v2.y);

  int delta_v0_col = v1.y - v2.y;
  int delta_v1_col = v2.y - v0.y;
  int delta_v2_col = v0.y - v1.y;

  int delta_v0_row = v2.x - v1.x;
  int delta_v1_row = v0.x - v2.x;
  int delta_v2_row = v1.x - v0.x;

  int bias0 = is_top_left_edge(v1, v2) ? -1 : 0;
  int bias1 = is_top_left_edge(v2, v0) ? -1 : 0;
  int bias2 = is_top_left_edge(v0, v1) ? -1 : 0;

  vec2i p = {min_x, min_y};
  int w0_row = cross_edge(v1, v2, p) + bias0;
  int w1_row = cross_edge(v2, v0, p) + bias1;
  int w2_row = cross_edge(v0, v1, p) + bias2;

  for (int y = min_y; y <= max_y; ++y) {
    int w0 = w0_row;
    int w1 = w1_row;
    int w2 = w2_row;
    if (y >= 0 && y < canvas->height) {
      for (int x = min_x; x <= max_x; ++x) {
        bool is_inside = x >= 0 && x < canvas->width;
        if (is_inside && (w0 | w1 | w2) > 0) {
          float r0 = w0 / area;
          float r1 = w1 / area;
          float r2 = 1 - r0 - r1;

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
        w0 += delta_v0_col;
        w1 += delta_v1_col;
        w2 += delta_v2_col;
      }
    }
    w0_row += delta_v0_row;
    w1_row += delta_v1_row;
    w2_row += delta_v2_row;
  }
}

// Normalized device coordinate
vec2 project_3d_to_2d(vec3 p) {
  return (vec2) {p.x / p.z, p.y / p.z};
}

vec2i project_2d_to_screen(Canvas* canvas, vec2 p) {
  return (vec2i) {
    (int)(((p.x+1)/2)     * canvas->width),
    (int)((1 - (p.y+1)/2) * canvas->height),
  };
}

void canvas_fill_triangle_3d(Canvas* canvas, Vertex3D triangle[3]) {
  vec2  v0f = project_3d_to_2d(triangle[0].position);
  vec2  v1f = project_3d_to_2d(triangle[1].position);
  vec2  v2f = project_3d_to_2d(triangle[2].position);
  vec2i v0 = project_2d_to_screen(canvas, v0f);
  vec2i v1 = project_2d_to_screen(canvas, v1f);
  vec2i v2 = project_2d_to_screen(canvas, v2f);

  float z0 = 1.0 / triangle[0].position.z;
  float z1 = 1.0 / triangle[1].position.z;
  float z2 = 1.0 / triangle[2].position.z;

  float area = cross_edge(v0, v1, v2);

  int min_x = MIN(MIN(v0.x, v1.x), v2.x);
  int min_y = MIN(MIN(v0.y, v1.y), v2.y);
  int max_x = MAX(MAX(v0.x, v1.x), v2.x);
  int max_y = MAX(MAX(v0.y, v1.y), v2.y);

  int delta_v0_col = v1.y - v2.y;
  int delta_v1_col = v2.y - v0.y;
  int delta_v2_col = v0.y - v1.y;

  int delta_v0_row = v2.x - v1.x;
  int delta_v1_row = v0.x - v2.x;
  int delta_v2_row = v1.x - v0.x;

  int bias0 = is_top_left_edge(v1, v2) ? -1 : 0;
  int bias1 = is_top_left_edge(v2, v0) ? -1 : 0;
  int bias2 = is_top_left_edge(v0, v1) ? -1 : 0;

  vec2i p = {min_x, min_y};
  int w0_row = cross_edge(v1, v2, p) + bias0;
  int w1_row = cross_edge(v2, v0, p) + bias1;
  int w2_row = cross_edge(v0, v1, p) + bias2;

  for (int y = min_y; y <= max_y; ++y) {
    int w0 = w0_row;
    int w1 = w1_row;
    int w2 = w2_row;
    if (y >= 0 && y < canvas->height) {
      for (int x = min_x; x <= max_x; ++x) {
        bool is_inside = (
          (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)
        );
        if (is_inside && x >= 0 && x < canvas->width) {
          float r0 = w0 / area;
          float r1 = w1 / area;
          float r2 = 1 - r0 - r1;

          const float z     = z0*r0 + z1*r1 + z2*r2;
          const u32   index = y * canvas->width + x;
          if (z > canvas->depth_buffer[index]) {
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

            canvas->color_buffer[index] = color;
            canvas->depth_buffer[index] = z;
          }
        }
        w0 += delta_v0_col;
        w1 += delta_v1_col;
        w2 += delta_v2_col;
      }
    }
    w0_row += delta_v0_row;
    w1_row += delta_v1_row;
    w2_row += delta_v2_row;
  }
}

void canvas_fill_rect(Canvas* canvas, Rect rect, int color) {
  int min_x = MIN(rect.x, rect.x + rect.width);
  int min_y = MIN(rect.y, rect.y + rect.height);
  int max_x = MAX(rect.x, rect.x + rect.width);
  int max_y = MAX(rect.y, rect.y + rect.height);

  for (int y = min_y; y < max_y; ++y) {
    if (y >= 0 && y < canvas->height) {
      for (int x = min_x; x < max_x; ++x) {
        if (x >= 0 && x < canvas->width) canvas_put_pixel(canvas, x, y, color);
      }
    }
  }
}

void canvas_clear(Canvas* canvas, int color) {
  for (i32 y = 0; y < canvas->height; ++y) {
    for (i32 x = 0; x < canvas->width; ++x) {
      canvas_put_pixel(canvas, x, y, color);
    }
  }
  memset(canvas->depth_buffer, 0, sizeof(float) * canvas->width * canvas->height);
}

u32 canvas_width(Canvas* canvas)  { return canvas->width; }
u32 canvas_height(Canvas* canvas) { return canvas->height; }