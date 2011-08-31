/**
 * @file cv_basic.c
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

#include "cv_basic.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/******************************************************************************/

result create_image(pixel_image *dst, void *data, pixel_type type, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    // store data in image so we have the pointer for freeing the data
    // in case other parameters are invalid
    dst->data = data;
    if (dst->data == NULL) {
        return BAD_POINTER;
    }

    if (width <= 0 || height <= 0 || dx < 0 || dy < 0) {
        return BAD_PARAM;
    }
    if (((dx + width) * step > stride) || ((dy + height) * stride > size)) {
        return BAD_PARAM;
    }

    dst->type = type;
    dst->dx = dx;
    dst->dy = dy;
    dst->width = width;
    dst->height = height;
    dst->offset = offset;
    dst->step = step;
    dst->stride = stride;
    dst->size = size;

    if (dst->step == 1) {
        dst->format = GREY;
    }
    else if (dst->step == 3) {
        dst->format = RGB3;
    }

    return SUCCESS;
}

/******************************************************************************/

result destroy_image(pixel_image *dst)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    if (dst->data != NULL) {
        free(dst->data);
        dst->data = NULL;
    }
    return SUCCESS;
}

/******************************************************************************/

result clone_image(pixel_image *dst, pixel_image *src)
{
    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->width <= 0 || src->height <= 0 || src->step <= 0) {
        return BAD_SIZE;
    }

    switch (src->type) {
    case U8:
        return allocate_byte_image(dst, src->width, src->height, src->step);
    case S8:
        return allocate_char_image(dst, src->width, src->height, src->step);
    case U16:
        return allocate_word_image(dst, src->width, src->height, src->step);
    case S32:
        return allocate_long_image(dst, src->width, src->height, src->step);
    case F64:
        return allocate_double_image(dst, src->width, src->height, src->step);
    default:
        return BAD_TYPE;
    }

    dst->format = src->format;

    return SUCCESS;
}

/******************************************************************************/

result copy_image(pixel_image *dst, pixel_image *src)
{
    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != dst->type || src->format != dst->format) {
        return BAD_TYPE;
    }
    if (src->width != dst->width || src->height != dst->height || src->step != dst->step) {
        return BAD_SIZE;
    }

    memcpy(dst->data, src->data, src->size);

    return SUCCESS;
}

/******************************************************************************/


/*
create_*_image
Can be used to create a subset of an existing image

dst: destination image structure to fill in
data: pointer to the image data array
dx: x coordinate for the top left corner of image rect
dy: y coordinate for the top left corner of image rect
width: image rect width
height: image rect height
offset: for multi-channel image (step > 1) the offset tells which channel is processed
step: number of elements (channels) per image pixel (offset to next col)
stride: number of elements per image row (offset to next row)
size: total size of image array (to determine number of rows in image)
*/
result create_byte_image(pixel_image *dst, byte *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    return create_image(dst, (void *)data, U8, dx, dy, width, height, offset, step, stride, size);
}

/******************************************************************************/

result create_char_image(pixel_image *dst, char *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    return create_image(dst, (void *)data, S8, dx, dy, width, height, offset, step, stride, size);
}

/******************************************************************************/

result create_word_image(pixel_image *dst, word *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    return create_image(dst, (void *)data, U16, dx, dy, width, height, offset, step, stride, size);
}

/******************************************************************************/

result create_long_image(pixel_image *dst, long *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    return create_image(dst, (void *)data, S32, dx, dy, width, height, offset, step, stride, size);
}

/******************************************************************************/

result create_double_image(pixel_image *dst, double *data, long dx, long dy, long width, long height, long offset, long step, long stride, long size)
{
    return create_image(dst, (void *)data, F64, dx, dy, width, height, offset, step, stride, size);
}

/******************************************************************************/

/*
allocate_*_image
Can be used to allocate a new image array
*/
result allocate_byte_image(pixel_image *dst, long width, long height, long step)
{
    long stride, size;
    byte *data;

    // must check that pointer to dst is valid
    // otherwise we don't have a place to store the pointer to allocated memory
    if (dst == NULL) {
        return BAD_POINTER;
    }

    stride = width * step;
    size = height * stride;
    data = (byte *)malloc((unsigned)size * sizeof(byte));

    return create_byte_image(dst, data, 0, 0, width, height, 0, step, stride, size);
}

/******************************************************************************/

result allocate_char_image(pixel_image *dst, long width, long height, long step)
{
    long stride, size;
    char *data;

    // must check that pointer to dst is valid
    // otherwise we don't have a place to store the pointer to allocated memory
    if (dst == NULL) {
        return BAD_POINTER;
    }

    stride = width * step;
    size = height * stride;
    data = (char *)malloc((unsigned)size * sizeof(char));

    return create_char_image(dst, data, 0, 0, width, height, 0, step, stride, size);
}

/******************************************************************************/

result allocate_word_image(pixel_image *dst, long width, long height, long step)
{
    long stride, size;
    word *data;

    // must check that pointer to dst is valid
    // otherwise we don't have a place to store the pointer to allocated memory
    if (dst == NULL) {
        return BAD_POINTER;
    }

    stride = width * step;
    size = height * stride;
    data = (word *)malloc((unsigned)size * sizeof(word));

    return create_word_image(dst, data, 0, 0, width, height, 0, step, stride, size);
}

/******************************************************************************/

result allocate_long_image(pixel_image *dst, long width, long height, long step)
{
    long stride, size;
    long *data;

    // must check that pointer to dst is valid
    // otherwise we don't have a place to store the pointer to allocated memory
    if (dst == NULL) {
        return BAD_POINTER;
    }

    stride = width * step;
    size = height * stride;
    data = (long *)malloc((unsigned)size * sizeof(long));

    return create_long_image(dst, data, 0, 0, width, height, 0, step, stride, size);
}

/******************************************************************************/

result allocate_double_image(pixel_image *dst, long width, long height, long step)
{
    long stride, size;
    double *data;

    // must check that pointer to dst is valid
    // otherwise we don't have a place to store the pointer to allocated memory
    if (dst == NULL) {
        return BAD_POINTER;
    }

    stride = width * step;
    size = height * stride;
    data = (double *)malloc((unsigned)size * sizeof(double));

    return create_double_image(dst, data, 0, 0, width, height, 0, step, stride, size);
}

/******************************************************************************/

/*
create_image_integrals
Initializes the structure and allocates the integral images
The integrals themselves are not calculated
Use calculate_integrals to create the actual integral images
*/
result create_integral_image(integral_image *dst, pixel_image *src)
{
    result r;
    if (dst == NULL || src == NULL || src->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U8) {
        return BAD_TYPE;
    }
    dst->original = src;
    dst->width = src->width;
    dst->height = src->height;
    dst->step = src->step;
    (*dst).integral.data = NULL;
    (*dst).integral2.data = NULL;
    r = allocate_long_image(&dst->integral, dst->width, dst->height, dst->step);
    if (r != SUCCESS) {
        return r;
    }
    r = allocate_double_image(&dst->integral2, dst->width, dst->height, dst->step);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

/******************************************************************************/

result destroy_integral_image(integral_image *dst)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    if (dst->original != NULL && dst->original->data != NULL) {
        free(dst->original->data);
        dst->original->data = NULL;
    }
    if ((*dst).integral.data != NULL) {
        free((*dst).integral.data);
        (*dst).integral.data = NULL;
    }
    if ((*dst).integral2.data != NULL) {
        free((*dst).integral2.data);
        (*dst).integral2.data = NULL;
    }
    return SUCCESS;
}

/******************************************************************************/

result clone_integral_image(integral_image *dst, integral_image *src)
{
    result r;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    /* TODO: need to consider how to handle image pointers in clone
    r = clone_image(dst->original, src->original);
    if (r != SUCCESS) {
        return r;
    }
    */
    /* ok at least when cloning for storing previous frame.. otherwise..? */
    dst->original = src->original;
    r = clone_image(&dst->integral, &src->integral);
    if (r != SUCCESS) {
        return r;
    }
    r = clone_image(&dst->integral2, &src->integral2);
    if (r != SUCCESS) {
        return r;
    }

    dst->width = src->width;
    dst->height = src->height;
    dst->step = src->step;

    return SUCCESS;
}

/******************************************************************************/

result copy_integral_image(integral_image *dst, integral_image *src)
{
    result r;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->width != dst->width || src->height != dst->height || src->step != dst->step) {
        return BAD_SIZE;
    }
    r = copy_image(dst->original, src->original);
    if (r != SUCCESS) {
        return r;
    }
    r = copy_image(&dst->integral, &src->integral);
    if (r != SUCCESS) {
        return r;
    }
    r = copy_image(&dst->integral2, &src->integral2);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
normalize
A function for normalizing a generic image
This means scaling the image values to 0-255 range
High-level function that resolves the type and calls appropriate function
*/
result normalize(const pixel_image *src, pixel_image *dst)
{
    if ((src->width != dst->width) || (src->height != dst->height)) {
        return BAD_SIZE;
    }
    if (dst->type != U8) {
        return BAD_TYPE;
    }
    switch (src->type) {
    case U8:
        return normalize_byte(src, dst, 0, 0, 0);
    case S8:
        return normalize_char(src, dst, 0, 0, 0);
    case U16:
        return normalize_word(src, dst, 0, 0, 0);
    case S32:
        return normalize_long(src, dst, 0, 0, 0);
    case F64:
        return normalize_double(src, dst, 0, 0, 0);
    default:
        return BAD_TYPE;
    }
}

/******************************************************************************/

/*
normalize_byte
Normalizes an image expressed in byte (unsigned char) values into unsigned char values
Maps smallest value to 0, largest to 255, and scales linearly values in between
Smallest, largest, and mean value can be given; if they are not given, they are
calculated.
*/
result normalize_byte(const pixel_image *src, pixel_image *dst, byte min, byte max, byte mean)
{
    const byte *data, *pos, *end;
    unsigned char *dstpos;
    //long max, min;
    double factor;
    int value;

    data = (byte *)src->data;
    if (min == 0 && max == 0) {
        min = *data;
        max = *data;
        pos = data + 1;
        end = data + src->size;
        while (pos < end) {
            if (*pos < min) {
                min = *pos;
            }
            else if (*pos > max) {
                max = *pos;
            }
            pos++;
        }
    }
    factor = 256.0 / (double)(max - min);

    pos = data;
    end = data + src->size;
    dstpos = (byte *)dst->data;
    while (pos < end) {
        value = (int)((double)(*pos - min) * factor);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        *dstpos = (byte)value;
        pos++;
        dstpos++;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
normalize_byte
Normalizes an image expressed in long integer values into unsigned char values
Maps smallest value to 0, largest to 255, and scales linearly values in between
Smallest, largest, and mean value can be given; if they are not given, they are
calculated.
*/
result normalize_char(const pixel_image *src, pixel_image *dst, char min, char max, char mean)
{
    const char *data, *pos, *end;
    unsigned char *dstpos;
    //long max, min;
    double factor;
    int value;

    data = (char *)src->data;
    if (min == 0 && max == 0) {
        min = *data;
        max = *data;
        pos = data + 1;
        end = data + src->size;
        while (pos < end) {
            if (*pos < min) {
                min = *pos;
            }
            else if (*pos > max) {
                max = *pos;
            }
            pos++;
        }
    }
    factor = 256.0 / (double)(max - min);

    pos = data;
    end = data + src->size;
    dstpos = (byte *)dst->data;
    while (pos < end) {
        value = (int)((double)(*pos - min) * factor);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        *dstpos = (byte)value;
        pos++;
        dstpos++;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
normalize_byte
Normalizes an image expressed in word (unsigned short) values into unsigned char values
Maps smallest value to 0, largest to 255, and scales linearly values in between
Smallest, largest, and mean value can be given; if they are not given, they are
calculated.
*/
result normalize_word(const pixel_image *src, pixel_image *dst, word min, word max, word mean)
{
    const word *data, *pos, *end;
    unsigned char *dstpos;
    //long max, min;
    double factor;
    int value;

    data = (word *)src->data;
    if (min == 0 && max == 0) {
        min = *data;
        max = *data;
        pos = data + 1;
        end = data + src->size;
        while (pos < end) {
            if (*pos < min) {
                min = *pos;
            }
            else if (*pos > max) {
                max = *pos;
            }
            pos++;
        }
    }
    factor = 256.0 / (double)(max - min);

    pos = data;
    end = data + src->size;
    dstpos = (byte *)dst->data;
    while (pos < end) {
        value = (int)((double)(*pos - min) * factor);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        *dstpos = (byte)value;
        pos++;
        dstpos++;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
normalize_long
Normalizes an image expressed in long integer values into unsigned char values
Maps smallest value to 0, largest to 255, and scales linearly values in between
*/
result normalize_long(const pixel_image *src, pixel_image *dst, long min, long max, long mean)
{
    const long *data, *pos, *end;
    unsigned char *dstpos;
    double factor;
    int value;

    data = (long *)src->data;
    if (min == 0 && max == 0) {
        min = *data;
        max = *data;
        pos = data + 1;
        end = data + src->size;
        while (pos < end) {
            if (*pos < min) {
                min = *pos;
            }
            else if (*pos > max) {
                max = *pos;
            }
            pos++;
        }
    }
    factor = 256.0 / (double)(max - min);

    pos = data;
    end = data + src->size;
    dstpos = (byte *)dst->data;
    while (pos < end) {
        value = (int)((double)(*pos - min) * factor);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        *dstpos = (byte)value;
        pos++;
        dstpos++;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
normalize_double
Normalizes an image expressed in double values into unsigned char values
Maps smallest value to 0, largest to 255, and scales linearly values in between
*/
result normalize_double(const pixel_image *src, pixel_image *dst, double min, double max, double mean)
{
    const double *data, *pos, *end;
    unsigned char *dstpos;
    double factor;
    int value;

    data = (double *)src->data;
    if (min == 0 && max == 0) {
        min = *data;
        max = *data;
        pos = data + 1;
        end = data + src->size;
        while (pos < end) {
            if (*pos < min) {
                min = *pos;
            }
            else if (*pos > max) {
                max = *pos;
            }
            pos++;
        }
    }
    factor = 256.0 / (max - min);

    pos = data;
    end = data + src->size;
    dstpos = (byte *)dst->data;
    while (pos < end) {
        value = (int)((*pos - min) * factor);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        *dstpos = (byte)value;
        pos++;
        dstpos++;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
convert_grey8_to_grey24
Turns a one-channel greyscale image into a three-channel greyscale image
by cloning the channel; this way the image can be used as an RGB image.
*/
result convert_grey8_to_grey24(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    byte *dst_data;
    long srcIndex, dstIndex, width, height, size;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src and dst must be byte images
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    // src must have 1 channel and dst 3 channels
    if (src->step != 1 || dst->step != 3) {
        return BAD_TYPE;
    }
    // src must be in greyscale and dst in RGB
    if (src->format != GREY || dst->format != RGB3) {
        return BAD_TYPE;
    }
    // src and dst must have same width
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;
    size = src->size;

    for (srcIndex = 0, dstIndex = 0; srcIndex < size; srcIndex += 1, dstIndex += 3) {
        dst_data[dstIndex] = dst_data[dstIndex + 1] = dst_data[dstIndex + 2] = src_data[srcIndex];
    }

    return SUCCESS;
}

/******************************************************************************/

result convert_rgb24_to_grey8(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    byte *dst_data;
    long srcIndex, dstIndex, width, height, size, value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src and dst must be byte images
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    // src must have 3 channel and dst 1 channels
    if (src->step != 3 || dst->step != 1) {
        return BAD_TYPE;
    }
    // src must be in RGB and dst in greyscale
    if (src->format != RGB3 || dst->format != GREY) {
        return BAD_TYPE;
    }
    // src and dst must have same width
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;
    size = src->size;

    for (srcIndex = 0, dstIndex = 0; srcIndex < size; srcIndex += 3, dstIndex += 1) {
        value = (int)(0.3 * src_data[srcIndex] + 0.59 * src_data[srcIndex + 1] + 0.11 * src_data[srcIndex + 2]);
        value = (value < 0) ? 0 : ((value > 255) ? 255 : value);
        dst_data[dstIndex] = (byte)value;
    }

    return SUCCESS;
}

/******************************************************************************/

result convert_grey8_to_radar(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    byte *dst_data;
    byte value;
    long row, col, srcIndex, dstIndex, width, height, size;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src and dst must be byte images
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    // src and dst must have 1 channel
    if (src->step != 1 || dst->step != 1) {
        return BAD_TYPE;
    }
    // src and dst must be in greyscale
    if (src->format != GREY || dst->format != GREY) {
        return BAD_TYPE;
    }
    // src and dst must have same width and dst must have height of 256
    if (src->width != dst->width || dst->height != 256) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;
    size = src->size;

    memset(dst_data, 0, dst->size);
    for (col = 0; col < width; col++) {
        srcIndex = col;
        for (row = 0; row < height; row++) {
            value = src_data[srcIndex];
            dstIndex = (255 - value) * width + col;
            if (dst_data[dstIndex] < 255) dst_data[dstIndex]++;
            srcIndex += width;
        }
    }

    return SUCCESS;
}

/******************************************************************************/

/*
scale_down
Scales the image down by factor of two without smoothing
Takes every other pixel on every other row
Supports using same image for both src and dst
*/
result scale_down(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    byte *dst_data;
    long row, col, width, height, old_pos, new_pos;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src and dst must be byte images
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    // TODO: src and dst have different size, need to determine how to check
    if (src->width * src->height * src->step > 2 * dst->size) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;

    old_pos = 0;
    new_pos = 0;

    for (row = 0; row < height; row += 2) {
        for (col = 0; col < width; col += 2) {
            dst_data[new_pos] = src_data[old_pos];
            old_pos += 2;
            new_pos += 1;
        }
        old_pos += width;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
scale_up
Scales the image up by factor of two without interpolation
Each pixel becomes four pixels
Supports using same image for both src and dst
*/
result scale_up(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    byte *dst_data;
    long row, col, width, height, old_pos, new_pos, new_width, new_height;
    byte value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src and dst must be byte images
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    // TODO: src and dst have different size, need to determing how to check
    if (4 * src->width * src->height * src->step > dst->size) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;
    new_width = width * 2;
    new_height = height * 2;

    old_pos = width * height - 1;
    new_pos = (new_width) * (new_height) - 1;

    for (row = 0; row < height; row++) {
        for (col = 0; col < height; col++) {
            value = src_data[old_pos];
            dst_data[new_pos] = dst_data[new_pos - 1] = dst_data[new_pos - new_width] = dst_data[new_pos - new_width - 1] = value;
            old_pos -= 1;
            new_pos -= 2;
        }
        new_pos -= new_width;
    }

    return SUCCESS;
}

/******************************************************************************/

/*
calculate_integrals
creates integral and squared integral images from src

accepts U8 image as src, S32 image as integral, and F64 image as integral2
images must have same dimensions
*/
result calculate_integrals(integral_image *src)
{
    byte *src_data;
    long *integral_data;
    double *integral2_data;
    long intensity;
    long x, y, width, height, currentPos, leftPos, abovePos, excludePos;

    if (src == NULL) {
        return BAD_POINTER;
    }
    if (src->original == NULL || (*src).integral.data == NULL || (*src).integral2.data == NULL) {
        return BAD_POINTER;
    }

    src_data = (byte *)src->original->data;
    integral_data = (long *)(*src).integral.data;
    integral2_data = (double *)(*src).integral2.data;

    width = src->width;
    height = src->height;

    // initialize top left corner of integral images
    intensity = src_data[0];
    integral_data[0] = intensity;
    integral2_data[0] = (double)(intensity * intensity);

    // initialize topmost row of integral images
    leftPos = 0;
    currentPos = 1;
    while (currentPos < width) {
        intensity = src_data[currentPos];
        integral_data[currentPos] = integral_data[leftPos] + intensity;
        integral2_data[currentPos] = integral2_data[leftPos] + (double)(intensity * intensity);
        leftPos = currentPos;
        currentPos++;
    }

    // initialize leftmost column of integral images
    abovePos = 0;
    currentPos = width;
    for (y = 1; y < height; y++) {
        intensity = src_data[currentPos];
        integral_data[currentPos] = integral_data[abovePos] + intensity;
        integral2_data[currentPos] = integral2_data[abovePos] + (double)(intensity * intensity);
        abovePos = currentPos;
        currentPos += width;
    }

    // initialize rest of integral images
    // add value of this pixel and integrals from top and left
    // subtract integral from top left diagonal
    excludePos = 0;
    for (y = 1; y < height; y++) {
        leftPos = y * width;
        currentPos = leftPos + 1;
        abovePos = excludePos + 1;
        for (x = 1; x < width; x++) {
            intensity = src_data[currentPos];
            integral_data[currentPos] = (integral_data[abovePos] - integral_data[excludePos]) + integral_data[leftPos] + intensity;
            integral2_data[currentPos] = (integral2_data[abovePos] - integral2_data[excludePos]) + integral2_data[leftPos] + (double)(intensity * intensity);

            leftPos = currentPos;
            currentPos++;
            excludePos = abovePos;
            abovePos++;
        }
        excludePos = y * width;
    }

    return SUCCESS;
}

