/**
 * @file cvsu_kinect.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Operations for handling kinect data in the cvsu module.
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

#include "cvsu_kinect.h"

/* for memset */
#include <string.h>

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
    /* src and dst must be byte images */
    if (src->type != U8 || dst->type != U8) {
        return BAD_TYPE;
    }
    /* src and dst must have 1 channel */
    if (src->step != 1 || dst->step != 1) {
        return BAD_TYPE;
    }
    /* src and dst must be in greyscale */
    if (src->format != GREY || dst->format != GREY) {
        return BAD_TYPE;
    }
    /* src and dst must have same width and dst must have height of 256 */
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

/* x, y, z coordinates will be in cm */

result depth16_to_pointcloud(pixel_image *src, pixel_image *dst)
{
    word *src_data;
    double *dst_data;
    long width, height, i, j, srcPos, dstPos;
    double x, y, z;
    double minDistance = -10;
    double scaleFactor = 0.0021;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != U16 || dst->type != F64) {
        return BAD_TYPE;
    }
    if (src->step != 1 || dst->step != 3) {
        return BAD_TYPE;
    }
    if (src->width != dst->width || src->height != dst->height) {
        return BAD_SIZE;
    }

    width = src->width;
    height = src->height;
    src_data = (word *)src->data;
    dst_data = (double *)dst->data;

    srcPos = 0;
    dstPos = 0;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (i < 10) {
                z = 100;
            }
            else if (i < 20) {
                z = 200;
            }
            else if (i < 30) {
                z = 300;
            }
            else {
                z = 100.0 / (-0.00307 * src_data[srcPos++] + 3.33);
            }
            x = (width / 2.0 - j) * (z + minDistance) * scaleFactor;
            y = (i - height / 2.0) * (z + minDistance) * scaleFactor;
            dst_data[dstPos++] = x;
            dst_data[dstPos++] = y;
            dst_data[dstPos++] = z;
        }
    }

    return SUCCESS;
}

result pointcloud_to_radar_top(pixel_image *src, pixel_image *dst)
{
    double *src_data;
    byte *dst_data;
    long width, height, size, srcPos, xPos, yPos, dstPos, srcWidth, srcHeight;
    double value, minX, maxX, minZ, maxZ, rangeX, rangeZ, srcAspect, dstAspect;
    double x, z, dx, dy, scale;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != F64 || dst->type != U8) {
        return BAD_TYPE;
    }
    if (src->step != 3 || dst->step != 1) {
        return BAD_TYPE;
    }

    src_data = (double *)src->data;
    dst_data = (byte *)dst->data;
    size = src->size;
    width = dst->width;
    height = dst->height;

    srcPos = 0;
    minX = maxX = src_data[srcPos];
    srcPos += 2;
    minZ = maxZ = src_data[srcPos];
    srcPos++;
    while (srcPos < size) {
        value = src_data[srcPos];
        if (value < minX) {
            minX = value;
        }
        else if (value > maxX) {
            maxX = value;
        }
        srcPos += 2;
        value = src_data[srcPos];
        if (value < minZ) {
            minZ = value;
        }
        else if (value > maxZ) {
            maxZ = value;
        }
        srcPos++;
    }
    maxX = 150;
    minX = -150;
    maxZ = 300;
    minZ = 0;
    rangeX = maxX - minX;
    rangeZ = maxZ - minZ;

    srcAspect = rangeX / rangeZ;
    dstAspect = width / height;

    if (srcAspect >= dstAspect) {
        srcWidth = width;
        srcHeight = (long)(width / srcAspect);
        dx = 0;
        dy = (height - srcHeight) / 2;
    }
    else {
        srcWidth = (long)(height * srcAspect);
        srcHeight = height;
        dx = (width - srcWidth) / 2;
        dy = 0;
    }
    scale = srcWidth / rangeX;

    memset(dst_data, 0, dst->size);
    srcPos = 0;
    while (srcPos < size) {
        x = src_data[srcPos];
        srcPos += 2;
        z = src_data[srcPos];
        srcPos++;
        xPos = (long)((x - minX) * scale + dx);
        xPos = (xPos < 0 ? 0 : (xPos > width - 1 ? width - 1 : xPos));
        yPos = (long)((z - minZ) * scale + dy);
        yPos = (yPos < 0 ? 0 : (yPos > height - 1 ? height - 1 : yPos));
        if (xPos > 0 && yPos > 0 && xPos < width - 1 && yPos < height - 1) {
            xPos = width - xPos;
            yPos = height - yPos;
            dstPos = yPos * width + xPos;
            dst_data[dstPos]++;
        }
    }

    normalize_byte(dst, dst, 0, 0, 0);

    return SUCCESS;
}
