/**
 * @file cvsu_uyvy.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Support for uyvy images in cvsu module.
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_uyvy.h"

#include <string.h>

string convert_uyvy16_to_grey8_name = "convert_uyvy16_to_grey8";
string convert_uyvy16_to_yuv24_name = "convert_uyvy16_to_yuv24";
string convert_yuyv16_to_grey8_name = "convert_yuyv16_to_grey8";
string scale_uyvy16_2_uyvy16_x2_name = "scale_uyvy16_2_uyvy16_x2";

result convert_uyvy16_to_grey8(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* simply copy y values from uyvy image to greyscale image */
    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        /* y values are in second channel, so must apply offset 1 to source */
        FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(1, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }
    else {
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        uint32 offset, x, y;
        /* for discontinuous images offset is applied to row table */
        /* therefore must correct the offset in case it's other than 0 */
        offset = 1 - source->offset;
        FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(offset, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }

    FINALLY(convert_uyvy16_to_grey8);
    RETURN();
}

/******************************************************************************/

result convert_uyvy16_to_yuv24(
    const pixel_image *source,
    pixel_image *target
    )
{
    byte y1,y2,u,v;
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 3);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == YUV);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* simply copy y value from second channel */
    /* u and v values are stored only once for two columnes */
    /* therefore, must read two columns at once */
    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        FOR_2_CONTINUOUS_IMAGES()
        {
            u  = PIXEL_VALUE(source);
            y1 = PIXEL_VALUE_PLUS(source,1);
            source_pos += source_step;
            v  = PIXEL_VALUE(source);
            y2 = PIXEL_VALUE_PLUS(source,1);

            PIXEL_VALUE(target)        = y1;
            PIXEL_VALUE_PLUS(target,1) = u;
            PIXEL_VALUE_PLUS(target,2) = v;
            target_pos += target_step;
            PIXEL_VALUE(target)        = y2;
            PIXEL_VALUE_PLUS(target,1) = u;
            PIXEL_VALUE_PLUS(target,2) = v;
        }
    }
    else {
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        uint32 x, y;
        FOR_2_DISCONTINUOUS_IMAGES()
        {
            u  = PIXEL_VALUE(source);
            y1 = PIXEL_VALUE_PLUS(source,1);
            source_pos += source_step;
            v  = PIXEL_VALUE(source);
            y2 = PIXEL_VALUE_PLUS(source,1);

            PIXEL_VALUE(target)        = y1;
            PIXEL_VALUE_PLUS(target,1) = u;
            PIXEL_VALUE_PLUS(target,2) = v;
            target_pos += target_step;
            PIXEL_VALUE(target)        = y2;
            PIXEL_VALUE_PLUS(target,1) = u;
            PIXEL_VALUE_PLUS(target,2) = v;
        }
    }

    FINALLY(convert_uyvy16_to_yuv24);
    RETURN();
}

/******************************************************************************/

result convert_yuyv16_to_grey8(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 1);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == GREY);
    CHECK_PARAM(source->width == target->width);
    CHECK_PARAM(source->height == target->height);

    /* simply copy y values from uyvy image to greyscale image */
    if (pixel_image_is_continuous(source) && pixel_image_is_continuous(target)) {
        CONTINUOUS_IMAGE_VARIABLES(byte, byte);
        /* y values are in first channel, so must apply offset 0 to source */
        FOR_2_CONTINUOUS_IMAGES_WITH_OFFSET(0, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }
    else {
        DISCONTINUOUS_IMAGE_VARIABLES(byte, byte);
        uint32 offset, x, y;
        /* for discontinuous images offset is applied to row table */
        /* therefore must correct the offset in case it's other than 0 */
        offset = 0 - source->offset;
        FOR_2_DISCONTINUOUS_IMAGES_WITH_OFFSET(offset, 0)
        {
            PIXEL_VALUE(target) = PIXEL_VALUE(source);
        }
    }

    FINALLY(convert_yuyv16_to_grey8);
    RETURN();
}

/******************************************************************************/

result scale_uyvy16_2_uyvy16_x2(
    const pixel_image *source,
    pixel_image *target
    )
{
    TRY();

    CHECK_POINTER(source);
    CHECK_POINTER(target);
    CHECK_POINTER(source->data);
    CHECK_POINTER(target->data);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(target->type == p_U8);
    CHECK_PARAM(source->step == 2);
    CHECK_PARAM(target->step == 2);
    CHECK_PARAM(source->format == UYVY);
    CHECK_PARAM(target->format == UYVY);
    CHECK_PARAM(target->width == 2 * source->width);
    CHECK_PARAM(target->height == 2 * source->height);

    /* read image in 4 byte chunks (unsigned long) and write out 4 times */
    {
        IMAGE_WITH_STEP_VARIABLES(uint32, uint32);
        uint32 x, y, offset_1, offset_2, offset_3;
        offset_1 = 1;
        offset_2 = target->stride / 2;
        offset_3 = offset_2 + 1;
        /* y values are in second channel, so must apply offset 1 to source */
        FOR_2_IMAGES_WITH_STEP(1, 1, 2, 2)
        {
            PIXEL_VALUE(target) =
            PIXEL_VALUE_PLUS(target, offset_1) =
            PIXEL_VALUE_PLUS(target, offset_2) =
            PIXEL_VALUE_PLUS(target, offset_3) =
            PIXEL_VALUE(source);
        }
    }

    /* alternative, slower code which preserves order of columns */
    /*
    u = src[srcPos++];
    y1 = src[srcPos++];
    v = src[srcPos++];
    y2 = src[srcPos++];

    dst[dstPos++] = u;
    dst[dstPos++] = y1;
    dst[dstPos++] = v;
    dst[dstPos++] = y1;
    dst[dstPos++] = u;
    dst[dstPos++] = y2;
    dst[dstPos++] = v;
    dst[dstPos++] = y2;

    dst[dstNxt++] = u;
    dst[dstNxt++] = y1;
    dst[dstNxt++] = v;
    dst[dstNxt++] = y1;
    dst[dstNxt++] = u;
    dst[dstNxt++] = y2;
    dst[dstNxt++] = v;
    dst[dstNxt++] = y2;
    */

    FINALLY(scale_uyvy16_2_uyvy16_x2);
    RETURN();
}

result scale_gray8_2_uyvy16_xn(pixel_image *src, pixel_image *dst, int scale)
{
    byte *src_data;
    byte *dst_data;
    int i, j, row, col, width, height, srcPos, dstPos, colStep, rowStep;
    int offsetCount;
    int offset[100];
    unsigned char value;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL || dst->data == NULL) {
        return BAD_POINTER;
    }
    if (src->type != p_U8 || dst->type != p_U8) {
        return BAD_TYPE;
    }
    if (src->step != 1 || dst->step != 2) {
        return BAD_TYPE;
    }
    if (src->format != GREY || dst->format != UYVY) {
        return BAD_TYPE;
    }
    if (dst->width != scale * src->width || dst->height != scale * src->height) {
        return BAD_SIZE;
    }

    width = src->width;
    height = src->height;
    src_data = (byte *)src->data;
    dst_data = (byte *)dst->data;
    /* calculate the actual count of offset values */
    offsetCount = scale * scale;
    for (i = 0; i < scale; i++) {
        for (j = 0; j < scale; j++) {
            offset[j * scale + i] = j * scale * width * 2 + i * 2;
        }
    }
    colStep = scale * 2;
    rowStep = scale * scale * width * 2;

    memset(dst_data, 128, width * scale * height * scale * 2);
    /*row = 0;*/
    srcPos = 0;
    /*dstPos = 1;*/
    for (row = 0; row < height; row++) {
        /*col = 0;*/
        dstPos = row * rowStep + 1;
        for (col = 0; col < width; col++) {
            value = src_data[srcPos++];
            for (i = 0; i < offsetCount; i++) {
                dst_data[dstPos + offset[i]] = value;
            }
            dstPos += colStep;
            /*col++;*/
        }
        /*row++;*/
    }

    return SUCCESS;
}
