/**
 * @file cv_filter.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Basic image filters for the cv module.
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

#include "cv_filter.h"

/* for memcpy */
#include <string.h>

/*
threshold
Replaces values larger than t with 255 and values smaller than t with 0
*/
result threshold(pixel_image *src, pixel_image *dst, byte t)
{
    byte *src_data;
    byte *dst_data;
    long pos, width, height, size;
    byte value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;

    size = width * height;
    for (pos = 0; pos < size; pos++) {
        value = src_data[pos];
        if (value >= t) {
            dst_data[pos] = 255;
        }
        else {
            dst_data[pos] = 0;
        }
    }

    return SUCCESS;
}

/*
smooth_binomial
Smoothes the image using binomial filtering
Filter can be applied several times for better result
*/
result smooth_binomial(const pixel_image *src, pixel_image *dst, int passes)
{
    byte *src_data;
    byte *dst_data;
    long pass, row, col, width, height, pos;
    byte prev, curr, next;

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
    // src and dst must have same width
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;

    width = src->width;
    height = src->height;

    memcpy(dst_data, src_data, (size_t)(width * height));

    for (pass = 0; pass < passes; pass++) {
        for (row = 0; row < height; row++) {
            pos = row * width;
            prev = dst_data[pos];
            pos++;
            curr = dst_data[pos];
            next = dst_data[pos + 1];
            for (col = 1; col < width - 1; col++) {
                /*
                apply binomial mask [ 1/4 1/2 1/4 ]
                */
                dst_data[pos] = (byte)((prev >> 2) + (curr >> 1) + (next >> 2));
                prev = curr;
                curr = next;
                pos++;
                next = dst_data[pos + 1];
            }
        }
        for (col = 0; col < width; col++) {
            pos = col;
            prev = dst_data[pos];
            pos += width;
            curr = dst_data[pos];
            next = dst_data[pos + width];
            for (row = 1; row < height - 1; row++) {
                /*
                apply binomial mask [ 1/4 1/2 1/4 ]
                */
                dst_data[pos] = (byte)((prev >> 2) + (curr >> 1) + (next >> 2));
                prev = curr;
                curr = next;
                pos += width;
                next = dst_data[pos + width];
            }
        }
    }

    return SUCCESS;
}

/*
sobel_x filters image with horizontal sobel operator
abs_sobel_x does the same using absolute values

accepts U8 image as src and S32 image as dst
images must have same dimensions
*/
result sobel_x(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    long *dst_data;
    long sobeloffsets[9];
    long sobelmaskx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    long i, j, width, height, row, col, pos, value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src must be byte image, integral must be long image, integral2 must be double image
    if (src->type != U8 || dst->type != S32) {
        return BAD_TYPE;
    }
    // both images must have same dimensions
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (long *)dst->data;

    width = src->width;
    height = src->height;

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[(j + 1) * 3 + (i + 1)] = j * width + i;
        }
    }

    memset(dst_data, 0, (size_t)(width * height) * sizeof(long));
    for (row = 1; row < height - 1; row++) {
        pos = row * width + 1;
        for (col = 1; col < width - 1; col++) {
            value = 0;
            for (i = 0; i < 9; i++) {
                value += src_data[pos + sobeloffsets[i]] * sobelmaskx[i];
            }
            dst_data[pos] = value;
            pos++;
        }
    }

    return SUCCESS;
}

result abs_sobel_x(const pixel_image *src, pixel_image *dst)
{
    byte *src_data;
    long *dst_data;
    long sobeloffsets[9];
    long sobelmaskx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    long i, j, width, height, row, col, pos, value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    // src must be byte image, integral must be long image, integral2 must be double image
    if (src->type != U8 || dst->type != S32) {
        return BAD_TYPE;
    }
    // both images must have same dimensions
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (byte *)src->data;
    dst_data = (long *)dst->data;

    width = src->width;
    height = src->height;

    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            sobeloffsets[(j + 1) * 3 + (i + 1)] = j * width + i;
        }
    }

    memset(dst_data, 0, (size_t)(width * height) * sizeof(long));
    for (row = 1; row < height - 1; row++) {
        pos = row * width + 1;
        for (col = 1; col < width - 1; col++) {
            value = 0;
            for (i = 0; i < 9; i++) {
                value += src_data[pos + sobeloffsets[i]] * sobelmaskx[i];
            }
            value = (value < 0) ? -value : value;
            dst_data[pos] = value;
            pos++;
        }
    }

    return SUCCESS;
}

/*
extrema_x
Calculates the extremal values along horizontal scanlines

parameter acceptance criteria:
src and dst and their data must be valid pointers
src must be a long image (type S32)
dst must be a long image (type S32)
src and dst must have same dimensions
src and dst may be the same image
*/
result extrema_x(const pixel_image *src, pixel_image *dst)
{
    long *src_data;
    long *dst_data;
    long x, y, width, height, pos;
    long value, prev;
    bool is_rising, is_falling;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != S32 || dst->type != S32) {
        return BAD_TYPE;
    }
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    src_data = (long *)src->data;
    dst_data = (long *)dst->data;

    width = src->width;
    height = src->height;

    for (y = 0; y < height; y++) {
        pos = y * width + 2;
        if (src_data[pos - 1] < src_data[pos - 2]) {
            is_falling = true;
        }
        else {
            is_falling = false;
        }
        if (src_data[pos - 1] > src_data[pos - 2]) {
            is_rising = true;
        }
        else {
            is_rising = false;
        }
        prev = src_data[pos - 1];

        for (x = 2; x < width; x++) {
            value = src_data[pos];
            if (value < prev) {
                if (is_rising) {
                    dst_data[pos - 1] = value;
                    is_rising = false;
                }
                else {
                    dst_data[pos - 1] = 0;
                }
                is_falling = true;
            }
            else if (value > prev) {
                if (is_falling) {
                    dst_data[pos - 1] = value;
                    is_falling = false;
                }
                else {
                    dst_data[pos - 1] = 0;
                }
                is_rising = true;
            }
            else {
                dst_data[pos - 1] = 0;
            }
            pos += 1;
            prev = value;
        }
    }
    return SUCCESS;
}
