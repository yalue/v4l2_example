.PHONY: all clean

CFLAGSS = -O3 -Wall -Werror
SDL_FLAGS = $(shell sdl2-config --cflags) $(shell sdl2-config --libs)

all: v4l2_test sdl_camera

webcam_lib.o: webcam_lib.c webcam_lib.h
	gcc -c $(CFLAGS) webcam_lib.c -o webcam_lib.o

convert_yuyv.o: convert_yuyv.h convert_yuyv.c
	gcc -c $(CFLAGS) convert_yuyv.c -o convert_yuyv.o

v4l2_test: v4l2_test.c webcam_lib.o convert_yuyv.o
	gcc -fno-strict-aliasing $(CFLAGS) webcam_lib.o convert_yuyv.o \
		v4l2_test.c -o v4l2_test

sdl_camera: sdl_camera.c webcam_lib.o
	gcc $(CFLAGS) webcam_lib.o sdl_camera.c -o sdl_camera $(SDL_FLAGS)

clean:
	rm -f v4l2_test
	rm -f *.o
