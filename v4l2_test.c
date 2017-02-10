// This is a simple program which will capture a frame from a webcam.
//
// Usage:
//    ./v4l2_image <device path e.g. "/dev/video0">
//
// The above command will save a file to output.bmp, containing a snapshot from
// the specified camera.
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "webcam_lib.h"

static char* ErrorString(void) {
  return strerror(errno);
}

int main(int argc, char **argv) {
  WebcamInfo webcam;
  if (argc != 2) {
    printf("Usage: %s <device path e.g. \"/dev/video0\">\n", argv[0]);
    return 1;
  }
  if (!OpenWebcam(argv[1], &webcam)) {
    printf("Error opening webcam: %s\n", ErrorString());
    return 1;
  }
  if (!PrintCapabilityDetails(&webcam)) {
    printf("Error printing camera capabilities: %s\n", ErrorString());
    CloseWebcam(&webcam);
    return 1;
  }
  if (!PrintVideoFormatDetails(&webcam)) {
    printf("Error printing video format details: %s\n", ErrorString());
    CloseWebcam(&webcam);
    return 1;
  }
  CloseWebcam(&webcam);
  return 0;
}
