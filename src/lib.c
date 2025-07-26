/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : lib.c
 * Created at  : 2025-06-04
 * Updated at  : 2025-07-27
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include "lib.h"
#include <stdio.h>

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

INLINE int cross_edge(vec2i a, vec2i b, vec2i p) {
  vec2i ab = {b.x - a.x, b.y - a.y};
  vec2i ap = {p.x - a.x, p.y - a.y};

  return ab.x * ap.y - ab.y * ap.x;
}

INLINE void canvas_put_pixel(Canvas* canvas, u32 x, u32 y, Color color) {
  Color bg = {.raw = canvas->color_buffer[y * canvas->width + x]};
  canvas->color_buffer[y * canvas->width + x] = blend_color(bg, color).raw;
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

          int r = r0 * triangle[0].color.rgba.r + r1 * triangle[1].color.rgba.r + r2 * triangle[2].color.rgba.r;
          int g = r0 * triangle[0].color.rgba.g + r1 * triangle[1].color.rgba.g + r2 * triangle[2].color.rgba.g;
          int b = r0 * triangle[0].color.rgba.b + r1 * triangle[1].color.rgba.b + r2 * triangle[2].color.rgba.b;

          Color color = {
            .rgba = {
              .r = CLAMP(r, 0, 255),
              .g = CLAMP(g, 0, 255),
              .b = CLAMP(b, 0, 255),
              .a = 0xFF,
            }
          };

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
            int r = r0 * triangle[0].color.rgba.r + r1 * triangle[1].color.rgba.r + r2 * triangle[2].color.rgba.r;
            int g = r0 * triangle[0].color.rgba.g + r1 * triangle[1].color.rgba.g + r2 * triangle[2].color.rgba.g;
            int b = r0 * triangle[0].color.rgba.b + r1 * triangle[1].color.rgba.b + r2 * triangle[2].color.rgba.b;
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

void canvas_fill_rect(Canvas* canvas, Rect rect, Color color) {
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

#if AA <= 1
INLINE void fill_circle_without_aa(Canvas* canvas, Circle circle, Color color) {
  i32 r     = circle.radius;
  i32 min_x = circle.x - r;
  i32 min_y = circle.y - r;
  i32 max_x = circle.x + r;
  i32 max_y = circle.y + r;

  i32 r2 = r*r;

  for (i32 y = min_y; y < max_y; ++y) {
    if (y < 0 || y >= canvas->height) continue;

    for (i32 x = min_x; x < max_x; ++x) {
      if (x < 0 || x >= canvas->width) continue;

      i32 x1 = x - circle.x;
      i32 y1 = y - circle.y;
      if (x1*x1 + y1*y1 <= r2) {
        canvas_put_pixel(canvas, x, y, color);
      }
    }
  }
}
#endif

#if AA > 1
INLINE void fill_circle_aa(Canvas* canvas, Circle circle, Color color) {
  i32 r     = circle.radius;
  i32 min_x = circle.x - r;
  i32 min_y = circle.y - r;
  i32 max_x = circle.x + r;
  i32 max_y = circle.y + r;

  i32 aa_cx     = circle.x * AA;
  i32 aa_cy     = circle.y * AA;
  i32 aa_radius = r * AA;

  i32 r2 = aa_radius*aa_radius;

  for (i32 y = min_y; y < max_y; ++y) {
    if (y < 0 || y >= canvas->height) continue;

    for (i32 x = min_x; x < max_x; ++x) {
      if (x < 0 || x >= canvas->width) continue;

      i32 coverage = 0;
      for (int sy = 0; sy < AA; ++sy) {
        for (int sx = 0; sx < AA; ++sx) {
          i32 aa_x = x * AA + sx;
          i32 aa_y = y * AA + sy;

          i32 dx = aa_x - aa_cx;
          i32 dy = aa_y - aa_cy;

          if (dx*dx + dy*dy <= r2) {
            coverage++;
          }
        }
      }

      if (coverage > 0) {
        float alpha = (float)coverage / (AA*AA);
        Color col = color;
        col.rgba.a *= alpha;
        canvas_put_pixel(canvas, x, y, col);
      }
    }
  }
}
#endif

void canvas_fill_circle(Canvas* canvas, Circle circle, Color color) {
#if AA > 1
  fill_circle_aa(canvas, circle, color);
#else
  fill_circle_without_aa(canvas, circle, color);
#endif
}

void canvas_stroke_circle(Canvas* canvas, Circle circle, Color color) {
  i32 r     = circle.radius;
  i32 min_x = circle.x - r;
  i32 min_y = circle.y - r;
  i32 max_x = circle.x + r;
  i32 max_y = circle.y + r;

  canvas_put_pixel(canvas, min_x, circle.y, color);
  canvas_put_pixel(canvas, max_x, circle.y, color);
  canvas_put_pixel(canvas, circle.x, min_y, color);
  canvas_put_pixel(canvas, circle.x, max_y, color);

  i32 x = 0;
  i32 y = circle.radius;

  i32 decision_slope = 1 - r;

  while (x < y) {
    ++x;
    if (decision_slope < 0) {
      decision_slope += 2*x + 1;
    } else {
      --y;
      decision_slope += 2*x - 2*y + 1;
    }

    canvas_put_pixel(canvas, x + circle.x, y + circle.y, color);
    canvas_put_pixel(canvas, x + circle.x, circle.y - y, color);
    canvas_put_pixel(canvas, circle.x - x, y + circle.y, color);
    canvas_put_pixel(canvas, circle.x - x, circle.y - y, color);

    canvas_put_pixel(canvas, y + circle.x, x + circle.y, color);
    canvas_put_pixel(canvas, y + circle.x, circle.y - x, color);
    canvas_put_pixel(canvas, circle.x - y, x + circle.y, color);
    canvas_put_pixel(canvas, circle.x - y, circle.y - x, color);
  }
}

void canvas_clear(Canvas* canvas, Color color) {
  for (i32 y = 0; y < canvas->height; ++y) {
    for (i32 x = 0; x < canvas->width; ++x) {
      canvas_put_pixel(canvas, x, y, color);
    }
  }
  memset(canvas->depth_buffer, 0, sizeof(float) * canvas->width * canvas->height);
}

u32 canvas_width(Canvas* canvas)  { return canvas->width; }
u32 canvas_height(Canvas* canvas) { return canvas->height; }

Color blend_color(Color bg, Color fg) {
  if (fg.rgba.a == 0)   return bg;
  if (fg.rgba.a == 255) return fg;

  i32 alpha_factor = bg.rgba.a * (255 - fg.rgba.a) >> 8;
  i32 alpha = fg.rgba.a + alpha_factor;

  i32 red   = ((fg.rgba.r * fg.rgba.a) + (bg.rgba.r * alpha_factor)) / alpha;
  i32 green = ((fg.rgba.g * fg.rgba.a) + (bg.rgba.g * alpha_factor)) / alpha;
  i32 blue  = ((fg.rgba.b * fg.rgba.a) + (bg.rgba.b * alpha_factor)) / alpha;

  return (Color) {
    .rgba = {
      .r = red,
      .g = green,
      .b = blue,
      .a = alpha,
    }
  };
}