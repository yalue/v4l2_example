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
#include <linux/videodev.h>

// Holds information about the webcam, needed by the library functions. Do not
// directly modify the members of this struct.
typedef struct {
  int fd;
  struct v4l2_capability capabilities;
} WebcamInfo;

typedef struct {
  uint8_t format_name[32];
  
} VideoMode;

// Takes a device path (e.g. /dev/video0) and a pointer to a WebcamInfo struct
// to populate. Returns 0 on error.
int OpenWebcam(char *path, WebcamInfo *info);

// Closes the webcam, freeing any allocated resources.
void CloseWebcam(WebcamInfo *info);

// Prints information about the open webcam to stdout. Returns 0 on error.
int PrintCapabilityDetails(WebcamInfo *info);

// Takes a pointer to a WebcamInfo struct, a pointer to an array of VideoModes
// structs, the number of entries in the VideoModes array, and a pointer to an
// which will be set to the number of modes needed. If the modes argument is
// NULL, this will still set the modes_needed value to enable allocating an
// array of the right size. If it isn't needed, modes_needed can be set to
// NULL, in which case this will return an error if VideoModes is also NULL.
// Returns 0 on error and nonzero otherwise. This will *only* return
// single-plane discrete-frame modes.
int GetSupportedVideoModes(WebcamInfo *info, VideoMode *modes,
    int modes_count, int *modes_needed);

#endif  // WEBCAM_LIB_H
