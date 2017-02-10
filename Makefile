.PHONY: all clean

all: v4l2_test

webcam_lib.o: webcam_lib.c webcam_lib.h
	gcc -c -O3 -Wall -Werror webcam_lib.c -o webcam_lib.o

v4l2_test: v4l2_test.c webcam_lib.o
	gcc -O3 -Wall -Werror v4l2_test.c webcam_lib.o -o v4l2_test

clean:
	rm -f v4l2_test
	rm -f *.o
