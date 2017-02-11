.PHONY: all clean

CFLAGSS = -O3 -Wall -Werror
SDL_FLAGS = $(shell sdl2-config --cflags) $(shell sdl2-config --libs)

all: sdl_camera

webcam_lib.o: webcam_lib.c webcam_lib.h
	gcc -c $(CFLAGS) webcam_lib.c -o webcam_lib.o

convert_yuyv.o: convert_yuyv.c convert_yuyv.h

sdl_camera: sdl_camera.c webcam_lib.o convert_yuyv.o
	gcc $(CFLAGS) webcam_lib.o convert_yuyv.o sdl_camera.c -o sdl_camera \
		$(SDL_FLAGS)

clean:
	rm -f sdl_camera
	rm -f *.o
