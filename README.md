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
