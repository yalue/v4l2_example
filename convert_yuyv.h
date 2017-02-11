// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This library provides functionality for converting YUYV (aka YUV2) 2-byte
// color to RGBA color images.
#ifndef CONVERT_YUYV_H
#define CONVERT_YUYV_H
#include <stdint.h>

// Converts the YUYV buffer pointed to by input to the 4-byte RGBA buffer
// pointed to by output. Each image is w pixels wide and h pixels tall. The row
// pitches are the number of bytes in a row for each image. Normally, this will
// just be 2 * w for the YUYV input pitch and 4 * w for the RGBA output pitch.
// Returns 0 on error.
int ConvertYUYVToRGBA(uint8_t *input, uint8_t *output, int w, int h,
    int input_pitch, int output_pitch);

#endif  // CONVERT_YUYV_H
