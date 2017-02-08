.PHONY: all clean

all: v4l2_test

v4l2_test: v4l2_test.c
	gcc -O3 -Wall -Werror v4l2_test.c -o v4l2_test

clean:
	rm -f v4l2_test
