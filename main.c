/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name   : main.c
 * Created at  : 2025-05-24
 * Updated at  : 2025-05-25
 * Author      : jeefo
 * Purpose     :
 * Description :
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stb_image_write.h"

#define WIDTH   600
#define HEIGHT  600

int canvas[HEIGHT * WIDTH];

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

int cross_edge(vec2 a, vec2 b, vec2 p) {
  vec2 ab = {b.x - a.x, b.y - a.y};
  vec2 ap = {p.x - a.x, p.y - a.y};

  return ab.x * ap.y - ab.y * ap.x;
}

float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

void rasterize_triangle(Vertex triangle[3]) {
  vec2 v0 = triangle[0].point;
  vec2 v1 = triangle[1].point;
  vec2 v2 = triangle[2].point;

  float area = cross_edge(v0, v1, v2);

  int min_x = MIN(v0.x, MIN(v1.x, v2.x));
  int min_y = MIN(v0.y, MIN(v1.y, v2.y));
  int max_x = MAX(v0.x, MAX(v1.x, v2.x));
  int max_y = MAX(v0.y, MAX(v1.y, v2.y));

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

        int index = y * WIDTH + x;
        canvas[index] = color;
      }
    }
  }
}

int main(void) {
  // clear background
  for (size_t y = 0; y < HEIGHT; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      size_t index = y * WIDTH + x;
      canvas[index] = 0xFF181818;
    }
  }

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
  Vertex triangle[3] = {
    { {(float)WIDTH * 0.5 , (float)HEIGHT * 0.1} , {255 , 0   , 0} }   ,
    { {(float)WIDTH * 0.9 , (float)HEIGHT * 0.9} , {  0 , 255 , 0} }   ,
    { {(float)WIDTH * 0.1 , (float)HEIGHT * 0.9} , {  0 , 0   , 255} } ,
  };
  rasterize_triangle(triangle);

  // TODO: implement rasterizer rule

  stbi_write_png("image.png", WIDTH, HEIGHT, 4, canvas, 0);

  return 0;
}