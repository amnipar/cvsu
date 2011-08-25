/**
 * @file cv_basic.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Basic types and operations for the cv module.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CV_BASIC_H
#   define CV_BASIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum pixel_type_t {
    U8 = 0,     // unsigned char values (byte)
    S8,         // signed char values
    U16,        // unsigned 16-bit integer values
    S16,        // signed 16-bit integer values
    U32,        // unsigned 32-bit integer values
    S32,        // signed 32-bit integer values (long)
    U64,        // unsigned 64-bit integer values
    S64,        // signed 64-bit integer values
    F32,        // 32-bit floating point values (float)
    F64         // 64-bit floating point values (double)
} pixel_type;

typedef enum pixel_format_t {
    GREY = 0,   // one-channel greyscale image
    RGB3,       // three-channel image with RGB values
    RGBA4,      // four-channel image with RGBA values
    HSV3,       // three-channel image with HSV values
    LAB3,       // three-channel image with LAB values
    UYVY2       // two-channel image with UYVY values
} pixel_format;

typedef struct pixel_image_t {
    void *data;             // pointer to pixel data array
    pixel_type type;        // data type used to store pixel values
    pixel_format format;    // format used to store one pixel
    long dx;                // x coord for top left corner of ROI
    long dy;                // y coord for top left corner of ROI
    long width;             // width of ROI
    long height;            // height of ROI
    long offset;            // channel offset for multi-channel images
    long step;              // number of elements to next col (channels)
    long stride;            // number of elements to next row
    long size;              // total number of elements in image array
} pixel_image;

typedef struct integral_image_t {
    pixel_image *original;
    pixel_image integral;
    pixel_image integral2;
    long width;
    long height;
    long step;
} integral_image;

typedef long integral_type;
typedef double integral2_type;

/**
 * Checks image parameters and fills in the struct fields.
 * The data must have been allocated previously.
 * Use this for creating a subset of an existing image, created with allocate.
 * @see allocate_*_image
 */

result create_image(pixel_image *dst, void *data, pixel_type type, long dx, long dy, long width, long height, long offset, long step, long stride, long size);

/**
 * Deallocates the image data.
 * @see allocate_*_image
 */

result destroy_image(pixel_image *dst);

/**
 * Transform the dst image into a clone of src image.
 * Only the structure is cloned, for copying content, use the copy function.
 * @see copy_image
 */

result clone_image(pixel_image *dst, pixel_image *src);

/**
 * Copy the content of src image into dst image.
 * The two images must have the same structure.
 * @see clone_image
 */

result copy_image(pixel_image *dst, pixel_image *src);

result create_byte_image(pixel_image *dst, byte *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size);
result create_char_image(pixel_image *dst, char *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size);
result create_word_image(pixel_image *dst, word *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size);
result create_long_image(pixel_image *dst, long *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size);
result create_double_image(pixel_image *dst, double *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size);

result allocate_byte_image(pixel_image *dst, long width, long height, long step);
result allocate_char_image(pixel_image *dst, long width, long height, long step);
result allocate_word_image(pixel_image *dst, long width, long height, long step);
result allocate_long_image(pixel_image *dst, long width, long height, long step);
result allocate_double_image(pixel_image *dst, long width, long height, long step);

result create_integral_image(integral_image *dst, pixel_image *src);
result destroy_integral_image(integral_image *dst);

/**
 * Transform the dst integral_image into a clone of src integral_image.
 * Only the structure is cloned, for copying content, use the copy function.
 * @see copy_integral_image
 */

result clone_integral_image(integral_image *dst, integral_image *src);

/**
 * Copy the content of src integral_image into dst integral_image.
 * The two images must have the same structure.
 * @see clone_integral_image
 */

result copy_integral_image(integral_image *dst, integral_image *src);

/*
normalize
Normalizes and scales image values to byte value range (0-255)
accepts S32 (long) or F64 (double) pixel types for src and U8 (byte) for dst
width and height of src and dst must match
*/
result normalize(const pixel_image *src, pixel_image *dst);
result normalize_byte(const pixel_image *src, pixel_image *dst, byte min, byte max, byte mean);
result normalize_char(const pixel_image *src, pixel_image *dst, char min, char max, char mean);
result normalize_word(const pixel_image *src, pixel_image *dst, word min, word max, word mean);
result normalize_long(const pixel_image *src, pixel_image *dst, long min, long max, long mean);
result normalize_double(const pixel_image *src, pixel_image *dst, double min, double max, double mean);

result convert_grey8_to_grey24(const pixel_image *src, pixel_image *dst);
result convert_rgb24_to_grey8(const pixel_image *src, pixel_image *dst);

result convert_grey8_to_radar(const pixel_image *src, pixel_image *dst);

result scale_down(const pixel_image *src, pixel_image *dst);
result scale_up(const pixel_image *src, pixel_image *dst);

/*
calculate_integrals creates integral and squared integral images from src

accepts U8 image as src, S32 image as integral, and F64 image as integral2
images must have same dimensions
*/
result calculate_integrals(integral_image *src);

#ifdef __cplusplus
}
#endif

#endif // CV_BASIC_H
