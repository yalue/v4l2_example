V4L2 Example for Zed Camera
===========================

About
-----

This project demonstrates how to get video from webcams using only Linux's
native V4L2 (Video For Linux version 2) interface. It includes a basic C
library for managing webcams and getting video frames.

Viewing the Demo
----------------

First, install SDL2, which the demo depends on (the library itself doesn't
need SDL, though, so only do this if you care about the demo program):

`sudo apt install libsdl2-dev`

Run `make`, followed by `./sdl_camera /dev/video0`. This should print
information about the camera and show a window displaying a video feed from the
camera.

Library Usage
-------------

The webcam usage library is contained in `webcam_lib.h` and `webcam_lib.c`.
A full set of the intended API is available by reading `webcam_lib.h`, which
includes comments on how to use all of the functions. To use the library,
simply `#include <webcam_lib.h>` and ensure that `webcam_lib.c` (or a compiled
version of it) is provided to the compiler.

Library API Example
-------------------

```C
#include <stddef.h>
#include <stdio.h>
#incldue <stdlib.h>
#include "webcam_lib.h"

static void ErrorExit(char *message) {
  printf("%s\n", message);
  exit(1);
}

int main(void) {
  WebcamInfo webcam;
  WebcamResolution resolution;
  void *pixel_data = NULL;
  size_t pixel_data_size = 0;
  if (!OpenWebcam("/dev/video0", &webcam)) {
    ErrorExit("Failed opening webcam");
  }
  // This takes an array of resolutions, but we only care about getting the
  // first one.
  if (!GetSupportedResolutions(&webcam, &resolution, 1)) {
    ErrorExit("Failed getting resolution");
  }
  if (!SetResolution(&webcam, resolution.width, resolution.height)) {
    ErrorExit("Failed setting resolution");
  }
  if (!BeginLoadingNextFrame(&webcam)) {
    ErrorExit("Failed starting to load a frame");
  }
  if (!GetFrameBuffer(&webcam, &pixel_data, &pixel_data_size)) {
    ErrorExit("Failed getting pixel data");
  }
  // Now, pixel_data will contain the frame, in the YUYV (aka YUY2) format.
  // Either use it as-is, or maybe use ConvertYUYVToRGBA(...) to convert it to
  // the RGBA format. pixel_data does *not* need to be unmapped or freed--that
  // will be taken care of by CloseWebcam if necessary.

  // Finally, close the camera and clean up resources:
  CloseWebcam(&webcam);
  return 0;
}
```

