// Author: Nathan Otterness (otternes@cs.unc.edu)
//
// This library provides a function for converting pixels in YUYV into RGB.
#ifndef CONVERT_YUYV_H
#define CONVERT_YUYV_H
#include <stddef.h>
#include <stdint.h>

// Call this to convert the YUYV image, held in the input buffer, into 4-byte
// RBGA in the output buffer (which must be allocated already). The input_pitch
// and output_pitch fields indicate how many bytes to skip between rows of
// pixels in the input and output buffers, respectively. Generally, this should
// just be (w * 2) for input_pitch and (w * 4) for output_pitch. Returns 0 on
// error.
int ConvertYUYV(uint8_t *input, uint8_t *output, int w, int h,
    size_t input_pitch, size_t output_pitch);

#endif  // CONVERT_YUYV_H
