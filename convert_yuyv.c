// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This file holds the implementation for the functions defined in
// convert_yuyv.h.
#include <stdint.h>
#include "convert_yuyv.h"

// Converts v to an unsigned byte, clamping it between 0 and 255.
static uint8_t Clamp(float v) {
  if (v > 255) return 255;
  if (v < 0) return 0;
  return (uint8_t) v;
}

// Converts the first 4 YUYV bytes into the input buffer and writes the first 8
// bytes in the output buffer.
static void ConvertTwoPixels(uint8_t *input, uint8_t *output) {
  float y_1, y_2, u, v;
  uint8_t r1, g1, b1, r2, g2, b2;
  // The YUYV format contains 2 pixels in 4 bytes. The 2 pixels have separate
  // Y (luminosity) values, but share U and V components (chrominance).
  y_1 = input[0];
  u = input[1];
  y_2 = input[2];
  v = input[3];
  r1 = Clamp(1.164 * (y_1 - 16) + 1.596 * (v - 128));
  g1 = Clamp(1.164 * (y_1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
  b1 = Clamp(1.164 * (y_1 - 16) + 2.018 * (u - 128));
  r2 = Clamp(1.164 * (y_2 - 16) + 1.596 * (v - 128));
  g2 = Clamp(1.164 * (y_2 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
  b2 = Clamp(1.164 * (y_2 - 16) + 2.018 * (u - 128));
  output[0] = r1;
  output[1] = g1;
  output[2] = b1;
  output[3] = 0xff;
  output[4] = r2;
  output[5] = g2;
  output[6] = b2;
  output[7] = 0xff;
}

int ConvertYUYV(uint8_t *input, uint8_t *output, int w, int h,
    size_t input_pitch, size_t output_pitch) {
  int x, y;
  size_t bytes_after_input_row, bytes_after_output_row;
  uint8_t *current_input, *current_output;
  // Make sure the provided pitches can hold an entire row.
  if (((w * 2) > input_pitch) || ((w * 4) > output_pitch)) {
    return 0;
  }
  // Make sure the width is even, since we always calculate 2 X pixels at once.
  if (w & 1) return 0;
  // Calculate the number of bytes to skip after the end of each row, if any.
  bytes_after_input_row = input_pitch - (w * 2);
  bytes_after_output_row = output_pitch - (w * 4);
  // Track the base offsets of the pixels we're currently converting.
  current_input = input;
  current_output = output;

  // Loop over the image, calculating 2 pixels at a time.
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x += 2) {
      ConvertTwoPixels(current_input, current_output);
      current_input += 4;
      current_output += 8;
    }
    current_input += bytes_after_input_row;
    current_output += bytes_after_output_row;
  }
  return 1;
}
