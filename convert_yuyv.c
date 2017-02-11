// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This file contains the implementation of the functions defined in
// convert_yuyv.h.
#include <stdint.h>
#include "convert_yuyv.h"

// Converts v to a byte, clamping it between 0 and 255.
static uint8_t Clamp(float v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t) v;
}

// Reads the two pixels contained in the first four bytes of the input buffer
// and writes the two pixels located in the first 8 bytes of the output vuffer.
static void ConvertTwoPixels(uint8_t *input, uint8_t *output) {
  float y_1, y_2, u, v;
  uint8_t r1, g1, b1, r2, g2, b2;
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

int ConvertYUYVToRGBA(uint8_t *input, uint8_t *output, int w, int h,
    int input_pitch, int output_pitch) {
  int unused_input_row_bytes, unused_output_row_bytes;
  int x, y;
  if (input_pitch < (w * 2)) return 0;
  if (output_pitch < (w * 4)) return 0;
  // Since the pitch contains the number of bytes per row, calculate the number
  // of bytes to skip after each row of meaningful pixels.
  unused_input_row_bytes = input_pitch - (w * 2);
  unused_output_row_bytes = output_pitch - (w * 4);
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      ConvertTwoPixels(input, output);
      input += 4;
      output += 8;
    }
    input += unused_input_row_bytes;
    output += unused_output_row_bytes;
  }
  return 1;
}
