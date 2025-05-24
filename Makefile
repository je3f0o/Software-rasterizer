
run: rasterizer
	./rasterizer && feh --auto-zoom image.png

rasterizer: main.o stb_image_write.o
	gcc -Wall -Wextra main.o stb_image_write.o -o rasterizer

stb_image_write.o: stb_image_write.h
	gcc -Wall -Wextra -DSTB_IMAGE_WRITE_IMPLEMENTATION -x c -c stb_image_write.h -o stb_image_write.o

main.o: main.c
	gcc -Wall -Wextra -c main.c -o main.o