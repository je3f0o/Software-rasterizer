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
#include <assert.h>
#include "stb_image_write.h"
#endif

void render(const Canvas* canvas) {
  assert(canvas != NULL);
  stbi_write_png("image.png", canvas->width, canvas->height, 4, canvas->data, 0);
}

void canvas_draw_scene(Canvas* canvas) {
  // clear background
  canvas_clear(canvas, 0xFF181818);

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
    { {(float)canvas->width * 0.5 , (float)canvas->height * 0.1} , {255 , 0   , 0} }   ,
    { {(float)canvas->width * 0.9 , (float)canvas->height * 0.9} , {  0 , 255 , 0} }   ,
    { {(float)canvas->width * 0.1 , (float)canvas->height * 0.9} , {  0 , 0   , 255} } ,
  };
  canvas_fill_triangle(canvas, triangle);

  // TODO: add more render targets
  render(canvas);
}

int main(void) {
  Canvas* canvas = create_canvas(800, 600);
  canvas_draw_scene(canvas);
  destroy_canvas(canvas);
  return 0;
}