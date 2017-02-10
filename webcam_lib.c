// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This file implements the API defined in webcam_lib.h.
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "webcam_lib.h"

// Prints the meaning of the set flags in the v4l2_capability struct's flag
// fields.
static void PrintCapabilityFlagDetails(uint32_t flags) {
  if (flags == 0) {
    printf("  <none>\n");
    return;
  }
  if (flags & 1) {
    printf("  Single-planar video capture\n");
  }
  if (flags & 0x1000) {
    printf("  Multi-planar video capture\n");
  }
  if (flags & 2) {
    printf("  Single-planar video output\n");
  }
  if (flags & 0x2000) {
    printf("  Multi-planar video output\n");
  }
  if (flags & 0x4000) {
    printf("  Single-planar video-to-memory API\n");
  }
  if (flags & 0x8000) {
    printf("  Multi-planar video-to-memory API\n");
  }
  if (flags & 4) {
    printf("  Video overlay interface\n");
  }
  if (flags & 0x10) {
    printf("  Raw VBI capture interface\n");
  }
  if (flags & 0x20) {
    printf("  Raw VBI output interface\n");
  }
  if (flags & 0x40) {
    printf("  Sliced VBI capture interface\n");
  }
  if (flags & 0x80) {
    printf("  Sliced VBI output interface\n");
  }
  if (flags & 0x100) {
    printf("  RDS capture interface\n");
  }
  if (flags & 0x200) {
    printf("  Video output overlay (OSD) interface\n");
  }
  if (flags & 0x400) {
    printf("  Hardware frequency seeking\n");
  }
  if (flags & 0x800) {
    printf("  RDS output interface\n");
  }
  if (flags & 0x10000) {
    printf("  Receive RF-modulated video signals\n");
  }
  if (flags & 0x20000) {
    printf("  Audio inputs or outputs\n");
  }
  if (flags & 0x40000) {
    printf("  Radio receiver\n");
  }
  if (flags & 0x80000) {
    printf("  Emit RF-modulated audio or video signals\n");
  }
  if (flags & 0x100000) {
    printf("  SDR capture interface\n");
  }
  if (flags & 0x200000) {
    printf("  v4l2_pix_format fields available\n");
  }
  if (flags & 0x400000) {
    printf("  SDR output interface\n");
  }
  if (flags & 0x1000000) {
    printf("  read() or write() I/O\n");
  }
  if (flags & 0x4000000) {
    printf("  Streaming I/O\n");
  }
  if (flags & 0x10000000) {
    printf("  Touch device\n");
  }
  if (flags & 0x80000000) {
    printf("  device_caps provided\n");
  }
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
int PrintVideoFormatDetails(WebcamInfo *webcam) {
  struct v4l2_fmtdesc info;
  uint32_t current_index = 0;
  int result = 0;
  info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  printf("Available image formats:\n");
  while (1) {
    info.index = current_index;
    result = ioctl(webcam->fd, VIDIOC_ENUM_FMT, &info);
    if (result < 0) {
      if (errno == EINVAL) break;
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
    if (!PrintFormatFrameSizes(webcam->fd, info.pixelformat)) return 0;
    current_index++;
  }
  return 1;
}

int PrintCapabilityDetails(WebcamInfo *webcam) {
  struct v4l2_capability *caps = &(webcam->capabilities);
  printf("Device %s on %s, driver %s:\n", caps->card, caps->bus_info,
    caps->driver);
  printf("Device features:\n");
  PrintCapabilityFlagDetails(caps->capabilities);
  if (caps->capabilities & 0x80000000) {
    // Print features in info.device_caps but not info.capabilities, ignoring
    // the device_caps_provided field.
    printf("Device features via a different interface:\n");
    PrintCapabilityFlagDetails((caps->device_caps ^ caps->capabilities) &
      0x7fffffff);
  }
  return 1;
}

int OpenWebcam(char *path, WebcamInfo *webcam) {
  int fd = open(path, O_RDWR);
  memset(webcam, 0, sizeof(*webcam));
  if (fd < 0) return 0;
  if (ioctl(fd, VIDIOC_QUERYCAP, &(webcam->capabilities)) < 0) {
    close(fd);
    return 0;
  }
  webcam->fd = fd;
  return 1;
}

void CloseWebcam(WebcamInfo *webcam) {
  close(webcam->fd);
  memset(webcam, 0, sizeof(*webcam));
}

int GetSupportedYUYVResolutions(WebcamInfo *webcam,
    WebcamResolution *resolutions, int resolutions_count) {
  struct v4l2_frmsizeenum info;
  int result;
  uint32_t api_index;
  int output_index = 0;
  info.pixel_format = v4l2_fourcc('Y', 'U', 'Y', 'V');
  // Ensure that all resolutions are zeroed-out so if the array isn't totally
  // full, the unset resolutions will simply be 0x0.
  memset(resolutions, 0, sizeof(WebcamResolution) * resolutions_count);

  // The API index tracks all of the frame sizes, but the output index only
  // tracks discrete frame sizes.
  for (api_index = 0; ; api_index++) {
    if (output_index > resolutions_count) break;
    info.index = api_index;
    result = ioctl(webcam->fd, VIDIOC_ENUM_FRAMESIZES, &info);
    if (result < 0) {
      if (errno == EINVAL) break;
      return 0;
    }
    // Skip continuous frame types.
    if (info.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) continue;
    // Skip stepwise frame types.
    if (info.type == V4L2_FRMSIZE_TYPE_STEPWISE) continue;
    // This is an error--encountered a frame size not defined by the v4l2
    // spec (should never happen).
    if (info.type != V4L2_FRMSIZE_TYPE_DISCRETE) return 0;
    resolutions[output_index].width = info.discrete.width;
    resolutions[output_index].height = info.discrete.height;
    output_index++;
  }
  return 1;

}


