// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This library provides a basic wrapper around the V4L2 (Video for Linux 2)
// API, which is used, among other things, for interfacing with webcams on
// Linux.
//
// To use, first call OpenWebcam("/dev/<path>", ...). When the webcam is no
// longer needed, pass a pointer to the WebcamInfo struct populated by
// OpenWebcam() to CloseWebcam(...) to clean up resources.
#ifndef WEBCAM_LIB_H
#define WEBCAM_LIB_H
#include <linux/videodev2.h>
#include <stdint.h>

// Holds information about the webcam, needed by the library functions. Do not
// directly modify the members of this struct.
typedef struct {
  int fd;
  struct v4l2_capability capabilities;
} WebcamInfo;

// Holds the resolution of a single discrete frame of video.
typedef struct {
  uint32_t width;
  uint32_t height;
} WebcamResolution;

// Takes a device path (e.g. /dev/video0) and a pointer to a WebcamInfo struct
// to populate. Returns 0 on error.
int OpenWebcam(char *path, WebcamInfo *webcam);

// Closes the webcam, freeing any allocated resources.
void CloseWebcam(WebcamInfo *webcam);

// Prints information about the open webcam to stdout. Returns 0 on error.
int PrintCapabilityDetails(WebcamInfo *webcam);

// Prints information about the webcam's supported video formats to stdout.
// Returns 0 on error.
int PrintVideoFormatDetails(WebcamInfo *webcam);

// Gets the supported YUYV (4:2:2) resolutions from the webcam. This will only
// provide resolutions for single-planar, discrete frames. This takes a pointer
// to an array of WebcamResolution structs, and will fill in up to
// resolutions_count members. Any entries in the list beyond the number of
// available resolutions will be set to 0. This returns 0 on error.
int GetSupportedYUYVResolutions(WebcamInfo *webcam,
    WebcamResolution *resolutions, int resolutions_count);

#endif  // WEBCAM_LIB_H
