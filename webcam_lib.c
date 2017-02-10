// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This file implements the API defined in webcam_lib.h.
#include <fcntl.h>
#include <linux/videodev.h>
#include <stdlib.h>
#include <stdint.h>
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

int PrintCapabilityDetails(WebcamInfo *info) {
  if (!info) return 0;
  struct v4l2_capability *caps = &(info.capabilities);
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

int OpenWebcam(char *path, WebcamInfo *info) {
  int fd = open(path, O_RDWR);
  if (fd < 0) return 0;
  if (ioctl(fd, VIDIOC_QUERYCAP, &(info->capabilities)) < 0) {
    close(fd);
    return 0;
  }
  return 1;
}

void CloseWebcam(WebcamInfo *info) {
  close(info->fd);
  memset(info, 0, sizeof(*info));
}

int GetSupportedVideoModes(WebcamInfo *info, VideoMode *modes,
    int modes_count, int *modes_needed) {
  int needed = 0;
}
