V4L2 Example for Zed Camera
===========================

About
-----

This project demonstrates how to get the current image from the Zed stereo
camera using only Linux's native V4L2 (Video For Linux version 2) interface.

Usage
-----

First, install SDL2, which this project depends on:
`sudo apt install libsdl2-dev`

Run `make`, followed by `./sdl_camera /dev/video0`. This should print
information about the camera and show a window displaying a video feed from the
camera.

Converting YUYV to RGB
----------------------

Webcams often support YUYV color rather than RGB. This project currently
directly displays the YUYV image using SDL, which supports YUYV out-of-the-box.
Converting from YUYV to RGB requires converting 2 pixels at a time. Here was an
attempted algorithm at converting a YUYV to RGB image, which doesn't work and
is left here while I work on it:

```c
uint8_t Clamp(float v) {
  if (v > 255) return 255;
  if (v < 0) return 0;
  return (uint8_t) v;
}

// Only the y values change between the subsequent 2 pixels.
first_pixel_y = (float) bytes[0];
second_pixel_y = (float) bytes[2];
shared_u = (float) bytes[1];
shared_v = (float) bytes[2];

// Maybe try taking the -16 away from the y if it looks funny.
first_pixel_b = Clamp(1.164 * (first_pixel_y - 16) + 2.018 * (shared_u - 128));
first_pixel_g = Clamp(1.164 * (first_pixel_y - 16) - 0.813 * (shared_v - 128) -
  0.391 * (shared_u - 128));
first_pixel_r = Clamp(1.164 * (first_pixel_y - 16) + 1.596 * (shared_v - 128));

// Do the same for the second pixel, substituting first_pixel_y with
// second_pixel_y.
```
