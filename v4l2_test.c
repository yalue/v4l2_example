// This is a simple program which will capture a frame from a webcam.
//
// Usage:
//    ./v4l2_image <device path e.g. "/dev/video0">
//
// The above command will save a file to output.bmp, containing a snapshot from
// the specified camera.
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static char* ErrorString(void) {
  return strerror(errno);
}

// Takes a file descriptor for a device and a pixel format from the
// v4l2_fmtdesc struct and prints a list of frame sizes supported by the
// format. Returns 0 on error, 1 on success.
static int PrintFormatFrameSizes(int fd, uint32_t pixel_format) {
  struct v4l2_frmsizeenum info;
  int result;
  uint32_t current_index;
  info.pixel_format = pixel_format;
  for (current_index = 0; ; current_index++) {
    info.index = current_index;
    result = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &info);
    if (result < 0) {
      if (errno == EINVAL) break;
      printf("Error getting frame size: %s\n", ErrorString());
      return 0;
    }
    if (info.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
      printf("    Continuous frame size.\n");
      continue;
    }
    if (info.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
      printf("    Discrete %dx%d frames\n", info.discrete.width,
        info.discrete.height);
      continue;
    }
    printf("    Stepwise %d-%dx%d-%d frames\n", info.stepwise.min_width,
      info.stepwise.max_width, info.stepwise.min_height,
      info.stepwise.max_height);
  }
  return 1;
}

// Takes a file descriptor for the device, and prints information about the
// image formats it supports to stdout. Returns 0 on error, 1 on success.
static int PrintAvailableImageFormats(int fd) {
  struct v4l2_fmtdesc info;
  uint32_t current_index = 0;
  int result = 0;
  info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  printf("Available image formats:\n");
  while (1) {
    info.index = current_index;
    result = ioctl(fd, VIDIOC_ENUM_FMT, &info);
    if (result < 0) {
      if (errno == EINVAL) break;
      printf("Failed getting device format description: %s\n", ErrorString());
      return 0;
    }
    printf("  %s", info.description);
    if (info.flags != 0) {
      printf(" (");
      if (info.flags & 1) {
        printf("compressed");
        if (info.flags != 1) {
          printf(", ");
        }
      }
      if (info.flags & 2) {
        printf("emulated");
      }
      printf(")");
    }
    printf("\n");
    printf("  Supported frame sizes:\n");
    if (!PrintFormatFrameSizes(fd, info.pixelformat)) return 0;
    current_index++;
  }
  return 1;
}

int main(int argc, char **argv) {
  int fd = 0;
  if (argc != 2) {
    printf("Usage: %s <device path e.g. \"/dev/video0\">\n", argv[0]);
    return 1;
  }
  fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    printf("Error opening device path %s: %s.\n", argv[1], ErrorString());
    return 1;
  }
  if (!PrintDeviceCapabilities(fd)) {
    close(fd);
    return 1;
  }
  if (!PrintAvailableImageFormats(fd)) {
    close(fd);
    return 1;
  }
  close(fd);
  return 0;
}
