V4L2 Example for Zed Camera
===========================

About
-----

This project demonstrates how to get the current image from the Zed stereo
camera using only Linux's native V4L2 (Video For Linux version 2) interface.

Usage
-----

Run `make`, followed by `./v4l2_test /dev/video0`. This should print
information about the camera and save a file named output.bmp, containing the
current video frame.

Converting YUYV to RGB
----------------------

Converting from YUYV to RGB requires converting 2 pixels at a time. For each
pixel:

```c
uint8_t Clamp(float v);
/
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
```
