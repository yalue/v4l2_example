// This is a simple program which will capture a frame from a webcam.
//
// Usage:
//    ./v4l2_image <device path e.g. "/dev/video0">
//
// The above command will save a file to output.png, containing a snapshot from
// the specified camera.
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "convert_yuyv.h"
#include "miniz.h"
#include "webcam_lib.h"

static char* ErrorString(void) {
  return strerror(errno);
}

int main(int argc, char **argv) {
  WebcamInfo webcam;
  void *frame_buffer = NULL;
  size_t frame_size = 0;
  uint8_t *rgb_buffer = NULL;
  void *png_image = NULL;
  size_t png_bytes = 0;
  uint32_t w, h;
  FILE *output_file = NULL;
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
    goto error_exit;
  }
  if (!PrintVideoFormatDetails(&webcam)) {
    printf("Error printing video format details: %s\n", ErrorString());
    goto error_exit;
  }
  if (!SetResolution(&webcam, 1344, 376)) {
    printf("Error setting video resolution: %s\n", ErrorString());
    goto error_exit;
  }
  if (!LoadFrame(&webcam)) {
    printf("Error loading frame: %s\n", ErrorString());
    goto error_exit;
  }
  if (!GetFrameBuffer(&webcam, &frame_buffer, &frame_size)) {
    printf("Error getting frame buffer: %s\n", ErrorString());
    goto error_exit;
  }

  // Create a buffer for a 4-byte RGBA image.
  GetResolution(&webcam, &w, &h);
  rgb_buffer = (uint8_t *) malloc(w * h * 4);
  if (!rgb_buffer) {
    printf("Failed allocating memory for RGB image.\n");
    goto error_exit;
  }
  if (!ConvertYUYV((uint8_t *) frame_buffer, rgb_buffer, w, h, w * 2, w * 4)) {
    printf("Failed converting image to RGB.\n");
    goto error_exit;
  }

  // Save the RGBA image as a PNG
  png_image = tdefl_write_image_to_png_file_in_memory_ex(rgb_buffer, w, h, 4,
    &png_bytes, MZ_BEST_COMPRESSION, 0);
  if (!png_image) {
    printf("Failed creating PNG image.\n");
    goto error_exit;
  }
  output_file = fopen("./output.png", "wb");
  if (!output_file) {
    printf("Failed opening the output file.\n");
    goto error_exit;
  }
  if (fwrite(png_image, 1, png_bytes, output_file) != png_bytes) {
    printf("Failed writing the output image.\n");
    goto error_exit;
  }
  printf("output.png saved.\n");

  fclose(output_file);
  mz_free(png_image);
  free(rgb_buffer);
  CloseWebcam(&webcam);
  return 0;
error_exit:
  CloseWebcam(&webcam);
  if (rgb_buffer) free(rgb_buffer);
  if (png_image) mz_free(png_image);
  if (output_file) fclose(output_file);
  return 1;
}
